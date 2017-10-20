#include "file_to_send.h"
#include "printer.hpp"
#include "errno.h"
#include "excp.h"

FileToSend::FileToSend (string _path, size_t first_byte_pos, size_t last_byte_pos):
		path (_path),
		first_byte (first_byte_pos),
		last_byte (last_byte_pos) 
{
	// открываем файл
	fp = fopen(path.c_str(), "r");
	// если ошибка с открытием
	if (fp == NULL) {
		Printer::error ("не могу открыть файл", "[FileToSend::FileToSend "+_path+"]");
		throw ClientException (strerror (errno));
	}
	// файл открылся нормально
	// узнаем размер
	fseek(fp, 0L, SEEK_END);
	file_size = ftell (fp);
	fseek (fp, 0L, SEEK_SET);
	// устанавливаем разные счетчики
	portion_size = last_byte - first_byte + 1;
	current_pos = first_byte;
	bytes_left = portion_size;
	// устанавливаем файл на нужную позицию
	fseek (fp, first_byte, SEEK_SET);
}

size_t FileToSend::fread (size_t n_bytes, char *buf) {
	// если просят прочитать больше, чем осталось
	if (n_bytes > bytes_left)
		// меняем на то, сколько осталось
		n_bytes = bytes_left;
	size_t n_read = ::fread (buf, sizeof (char), n_bytes, fp);
	// если прочитали меньше, чем потребовали
	if (n_read < n_bytes)
		throw FileException ("Прочитали меньше, чем нужно");
	// прочитали n_bytes
	// увеличиваем указатели и т.п.
	current_pos += n_bytes;
	bytes_left -= n_bytes;
	return n_read;
}

void FileToSend::rewind_back (size_t n_bytes) {
	if (n_bytes > current_pos - first_byte)
		n_bytes = current_pos - first_byte;
	fseek (fp, current_pos - n_bytes, SEEK_SET);
}