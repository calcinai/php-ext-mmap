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


php_stream_ops mmap_ops = {	mmap_write,	mmap_read,	mmap_close,	mmap_flush,	"mmap stream", /* stream type name */	mmap_seek, 
	NULL, 
	NULL, 
	NULL};



size_t mmap_write(php_stream * stream, char *buf, size_t count TSRMLS_DC) {	int wrote;	struct mmap_stream_data *data = stream->abstract;	wrote = MIN(data->base_pos + data->len - data->current_pos, count); 
	if(wrote == 0) {		return 0; 
	}	memcpy(data->current_pos, buf, wrote); data->current_pos += wrote;	return wrote;}

size_t mmap_read(php_stream *stream, char *buf, size_t count TSRMLS_DC) {	int to_read;	struct mmap_stream_data *data = stream->abstract;	to_read = MIN(data->base_pos + data->len - data->current_pos, count); 
	if(to_read == 0) {		return 0; 
	}	memcpy(buf, data->current_pos, to_read); data->current_pos += to_read;	return to_read;}int mmap_flush(php_stream *stream TSRMLS_DC) {	struct mmap_stream_data *data = stream->abstract;	return msync(data->base_pos, data->len, MS_SYNC | MS_INVALIDATE); 
}


int mmap_seek(php_stream *stream, off_t offset, int whence, off_t *newoffset TSRMLS_DC) {	struct mmap_stream_data *data = stream->abstract; 
	switch(whence) {		case SEEK_SET:			if(offset < 0 || offset > data->len) {				*newoffset = (off_t) -1;				return -1; 
			}			data->current_pos = data->base_pos + offset; *newoffset = offset;			return 0;			break;		case SEEK_CUR:			if(data->current_pos + offset < data->base_pos || data->current_pos + offset > data->base_pos + data->len){
				*newoffset = (off_t) -1;				return -1;			}			data->current_pos += offset;			*newoffset = data->current_pos - data->base_pos; 
			return 0;			break;		case SEEK_END:			if(offset > 0 || -1 * offset > data->len) {				*newoffset = (off_t) -1;				return -1; 
			}			data->current_pos += offset;			*newoffset = data->current_pos - data->base_pos; 
			return 0;			break;
		default:			*newoffset = (off_t) -1;			return -1; 
	}}

int mmap_close(php_stream *stream, int close_handle TSRMLS_DC) {	struct mmap_stream_data *data = stream->abstract;	if(close_handle) {
		munmap(data->base_pos, data->len);	} 
	efree(data); 
	return 0;}
PHP_FUNCTION(mmap_open) {	char *filename; 
	long filename_len; 
	long block_size; 
	long offset; 
	int fd;	php_stream * stream; void *mpos;	struct mmap_stream_data *data;	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sll", &filename, &filename_len, &block_size, &offset) == FAILURE){ 
		return;	}	if((fd = open(filename, O_RDWR)) < -1) {		RETURN_FALSE; 
	}
		if((mpos = mmap(NULL, block_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, offset)) == (void *) -1) {		close(fd);		RETURN_FALSE; 
	}	data = emalloc(sizeof(struct mmap_stream_data)); 
	data->base_pos = mpos;	data->current_pos = mpos;	data->len = block_size;	close(fd);	stream = php_stream_alloc(&mmap_ops, data, NULL, "r+"); 
	php_stream_to_zval(stream, return_value);}