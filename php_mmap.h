#ifndef PHP_MMAP
#define PHP_MMAP_H 1
#define PHP_MMAP_VERSION "1.0"
#define PHP_MMAP_EXTNAME "mmap"

PHP_FUNCTION(mmap_open);

extern zend_module_entry mmap_module_entry;
#define phpext_mmap_ptr &mmap_module_entry

size_t mmap_stream_write(php_stream *stream, const char *buffer, size_t length TSRMLS_DC);size_t mmap_stream_read(php_stream *stream, char *buffer, size_t length TSRMLS_DC);int mmap_stream_flush(php_stream *stream TSRMLS_DC);int mmap_stream_seek(php_stream *stream, zend_off_t offset, int whence, zend_off_t *newoffset);
int mmap_stream_close(php_stream *stream, int close_handle TSRMLS_DC);

struct mmap_stream_data { 
	void *base_offset;	void *current_offset;	int length;};

#endif
