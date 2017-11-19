#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_mmap.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

static zend_function_entry mmap_functions[] = {
    PHP_FE(mmap_open, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry mmap_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_MMAP_EXTNAME,
    mmap_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_MMAP_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_MMAP
ZEND_GET_MODULE(mmap)
#endif


php_stream_ops mmap_ops = {
    mmap_stream_write,
    mmap_stream_read,
    mmap_stream_close,
    mmap_stream_flush,
    "mmap", /* stream type name */
    mmap_stream_seek, 
    NULL, 
    NULL, 
    NULL
};



size_t mmap_stream_write(php_stream *stream, const char *buffer, size_t length TSRMLS_DC) {
    int to_write;
    struct mmap_stream_data *data = stream->abstract;
    
    to_write = MIN(data->base_offset + data->length - data->current_offset, length); 
    if(to_write == 0) {
        return 0; 
    }
    memcpy(data->current_offset, buffer, to_write); 
    data->current_offset += to_write;
    return to_write;
}

size_t mmap_stream_read(php_stream *stream, char *buffer, size_t length TSRMLS_DC) {
    int to_read;
    struct mmap_stream_data *data = stream->abstract;
    
    to_read = MIN(data->base_offset + data->length - data->current_offset, length); 
    if(to_read == 0) {
        return 0; 
    }
    memcpy(buffer, data->current_offset, to_read); 
    data->current_offset += to_read;
    return to_read;
}

int mmap_stream_flush(php_stream *stream TSRMLS_DC) {
    struct mmap_stream_data *data = stream->abstract;
    
    return msync(data->base_offset, data->length, MS_SYNC | MS_INVALIDATE); 
}


int mmap_stream_seek(php_stream *stream, zend_off_t offset, int whence, zend_off_t *newoffset) {
    struct mmap_stream_data *data = stream->abstract; 
    
    switch(whence) {
        case SEEK_SET:
            if(offset < 0 || offset > data->length) {
                *newoffset = (off_t) -1;
                return -1; 
            }
            data->current_offset = data->base_offset + offset; *newoffset = offset;
            return 0;
        case SEEK_CUR:
            if(data->current_offset + offset < data->base_offset || data->current_offset + offset > data->base_offset + data->length){
                *newoffset = (off_t) -1;
                return -1;
            }
            data->current_offset += offset;
            *newoffset = data->current_offset - data->base_offset; 
            return 0;
        case SEEK_END:
            if(offset > 0 || -1 * offset > data->length) {
                *newoffset = (off_t) -1;
                return -1; 
            }
            data->current_offset += offset;
            *newoffset = data->current_offset - data->base_offset; 
            return 0;
        default:
            *newoffset = (off_t) -1;
            return -1; 
    }
}


int mmap_stream_close(php_stream *stream, int close_handle TSRMLS_DC) {
    struct mmap_stream_data *data = stream->abstract;
    
    if(close_handle) {
        munmap(data->base_offset, data->length);
    } 
    efree(data); 
    return 0;
}

PHP_FUNCTION(mmap_open) {
    char *filename; 
    long filename_length; 
    long block_size; 
    long offset; 
    int fd;
    php_stream * stream; void *mem_pos;
    struct mmap_stream_data *data;
    
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &filename, &filename_length, &block_size, &offset) == FAILURE){ 
        return;
    }
    if((fd = open(filename, O_RDWR)) < -1) {
        RETURN_FALSE; 
    }
    
    if((mem_pos = mmap(NULL, block_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset)) == (void *) -1) {
        close(fd);
        RETURN_FALSE; 
    }
    
    data = emalloc(sizeof(struct mmap_stream_data)); 
    data->base_offset = mem_pos;
    data->current_offset = mem_pos;
    data->length = block_size;
    
    close(fd);
    stream = php_stream_alloc(&mmap_ops, data, NULL, "r+"); 
    php_stream_to_zval(stream, return_value);
}
