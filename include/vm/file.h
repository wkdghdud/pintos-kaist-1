#ifndef VM_FILE_H
#define VM_FILE_H
#include "filesys/file.h"
#include "vm/vm.h"

struct page;
enum vm_type;

struct file_page
{
	/* 연결된 파일 */
	struct file *file;
	/* 파일백드의 헤드 */
	void *addr;
	/* 현재 파일백드 페이지가 연결된 가상 주소 */
	void *upage;
	/* 현재 파일 백드 페이지의 길이 */
	size_t page_length;
	/* 전체 파일백드 페이지의 길이 */
	size_t total_length;
	/* 현재 파일백드 페이지에 해당하는 파일의 오프셋 */
	off_t offset;
};


void vm_file_init (void);
bool file_backed_initializer (struct page *page, enum vm_type type, void *kva);
void *do_mmap(void *addr, size_t length, int writable,
		struct file *file, off_t offset);
void do_munmap (void *va);

#endif
