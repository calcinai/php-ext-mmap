# php-ext-mmap
This is a native mmap extension for PHP.  It's been put together from a few bits and pieces and is designed to be a drop-in replacement for [php-mmap](https://github.com/calcinai/php-mmap) to improve performance.

## Installation
Installing custom php extensions is a pain, but if you have all the tools (php5-dev, gcc etc) installed, it's pretty easy

```
$ phpize
$ ./configure
$ sudo make install
```

Then it's just a matter of enabling the extension in your php.ini by adding the following line:

```extension=mmap.so```


## Usage

Calling the `mmap_open()` function will return a stream resource that can be used with `fread()`, `fwrite()`, `fseek()`, `fclose()`

```php
	//mmap_open($file_name, $block_size, $offset)
    $mmap = mmap_open('/dev/mem', 1024, 0);
```