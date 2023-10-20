/* file.c: Implementation of memory backed file object (mmaped object). */

#include "vm/vm.h"
#include "userprog/process.h"
#include <debug.h>
#include "include/threads/vaddr.h"
#include "include/threads/mmu.h"
#include "include/filesys/file.h"

static bool file_backed_swap_in (struct page *page, void *kva);
static bool file_backed_swap_out (struct page *page);
static void file_backed_destroy (struct page *page);
bool *do_lazy_mmap(struct page *page, void *aux);

void do_munmap (void *addr);
void* do_mmap (void *addr, size_t length, int writable, struct file *file, off_t offset);

/* DO NOT MODIFY this struct */
static const struct page_operations file_ops = {
	.swap_in = file_backed_swap_in,
	.swap_out = file_backed_swap_out,
	.destroy = file_backed_destroy,
	.type = VM_FILE,
};

/* The initializer of file vm */
void
vm_file_init (void) {
}

/* Initialize the file backed page */
bool
file_backed_initializer (struct page *page, enum vm_type type, void *kva) {
	/* Set up the handler */
	page->operations = &file_ops;

	struct file_page *file_page = &page->file;
}

/* Swap in the page by read contents from the file. */
static bool
file_backed_swap_in (struct page *page, void *kva) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Swap out the page by writeback contents to the file. */
static bool
file_backed_swap_out (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}

/* Destory the file backed page. PAGE will be freed by the caller. */
static void
file_backed_destroy (struct page *page) {
	struct file_page *file_page UNUSED = &page->file;
}


/* Do the mmap */

/*
// addr부터 시작하는 연속된 유저 가상 메모리 공간에 page들을 만들어 file의 offset부터 length에 해당하는 file의 정보를 각 page마다 저장한다. 
프로세스가 이 page에 접근해서 page fault를 발생시키면 physical frame과 mapping하여 (claim) disk에서 file data를 frame에 복사함

// 실질적으로 가상 페이지를 할당해주는 함수이다. 인자로 받은 addr부터 시작하는 연속적인 유저 가상 메모리 공간에 페이지를 생성해 
file의 offset부터 length에 해당하는 크기만큼 파일의 정보를 각 페이지마다 저장한다. 프로세스가 이 페이지에 접근해서 page fault가 
뜨면 물리 프레임과 매핑(이때 claim을 사용한다)해 디스크에서 파일 데이터를 프레임에 복사한다.

// 이전에 다뤘던 load_segment()와 유사한 로직이다. 차이점은

// 1) load_segment()가 파일의 정보를 담은 uninit 타입 페이지를 만들 때 파일 타입을 VM_FILE로 선언해주는 것과
// 2) 매핑이 끝난 후 연속된 유저 페이지 나열의 첫 주소를 리턴한다는 점이 있다.

// 성공하면 이 함수는 파일이 매핑된 가상 주소를 반환합니다. 실패하면 파일을 매핑하는 데 유효한 주소가 아닌 NULL을 반환해야 합니다.
*/
void *
do_mmap(void *addr, size_t length, int writable,
		struct file *file, off_t offset)
{
	struct thread *curr = thread_current();
	struct supplemental_page_table *spt = &curr->spt;
	uint64_t *upage = pg_round_down(addr);
	void *old_addr = addr;
	struct file *m_file = file_reopen(file);
	// ASSERT(pg_ofs(upage) == 0);
	// ASSERT(addr % PGSIZE == 0);
	size_t read_bytes = length;
	off_t ofs_curr = offset;
	while (read_bytes > 0)
	{
		size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
		struct lazy_mmap *aux = malloc(sizeof(struct lazy_mmap));
		//파일백드의 시작주소
		aux->offset = offset;
		aux->addr = addr;
		aux->upage = upage;
		aux->length = length;
		aux->writable = writable;
		aux->file = m_file;
		aux->offset = ofs_curr;
		aux->page_read_bytes = page_read_bytes;
		ofs_curr += page_read_bytes;
		if (!vm_alloc_page_with_initializer(VM_FILE,addr,
											writable, do_lazy_mmap, aux))
			free(aux);
		read_bytes -= page_read_bytes;
		addr += PGSIZE;
		//offset+= page_read_bytes;
		//return addr;
	}
	return old_addr;
}

/* Do the munmap */
/*
// memory unmapping을 실행한다. 즉, 페이지에 연결되어 있는 물리 프레임과의 연결을 끊어준다.
 유저 가상 메모리의 시작 주소 addr부터 연속으로 나열된 페이지 모두를 매핑 해제한다.

// 이때 페이지의 Dirty bit이 1인 페이지는 매핑 해제 전에 변경 사항을 디스크 파일에 업데이트해줘야 한다.
이를 위해 페이지의 container 구조체에서 연결된 파일에 대한 정보를 가져온다.
*/
void do_munmap (void *addr) {
	while(true){
		struct thread *curr = thread_current();
		struct page *find_page = spt_find_page(&curr->spt, addr);
	
		// struct frame *find_frame =find_page->frame;
		if (find_page == NULL) {
    		return NULL;
    	}
		// 연결 해제
		// find_page->frame = NULL;
		// find_frame->page = NULL;
		struct lazy_mmap *aux = (struct lazy_mmap* )find_page->uninit.aux;
		// 페이지의 dirty bit이 1이면 true를, 0이면 false를 리턴한다.
		if (pml4_is_dirty(curr->pml4, find_page->va)){
			// 물리 프레임에 변경된 데이터를 다시 디스크 파일에 업데이트 buffer에 있는 데이터를 size만큼, file의 file_ofs부터 써준다.
			

			// file_write_at(aux->file, addr, 4096, aux->offset);
			// printf("z\n");
			// printf("z\n");

			file_seek(find_page->file.file,find_page->file.offset);
			file_write(find_page->file.file, addr, find_page->file.page_length);
			// dirty bit = 0
			// 인자로 받은 dirty의 값이 1이면 page의 dirty bit을 1로, 0이면 0으로 변경해준다.
			pml4_set_dirty(curr->pml4,find_page->va,0);
		} else{
			pml4_clear_page(curr->pml4, find_page->va); 
			palloc_free_page(find_page->frame->kva);
			hash_delete(&curr->spt, &find_page->hash_elem);
			free(find_page);
		}
		// dirty bit = 0
		// 인자로 받은 dirty의 값이 1이면 page의 dirty bit을 1로, 0이면 0으로 변경해준다.
		// present bit = 0
		// 페이지의 present bit 값을 0으로 만들어주는 함수
		// pml4_clear_page(curr->pml4, find_page->va); 
		addr += PGSIZE;
	}
}

bool *do_lazy_mmap(struct page *page, void *aux)
{
	struct thread *curr = thread_current();
	struct lazy_mmap *lm = aux;
	struct file *file = lm->file;
	off_t ofs = lm->offset;
	bool writable = lm->writable;
	file_seek(file, ofs);
	size_t page_read_bytes = lm->page_read_bytes;
	uint8_t *kpage = page->frame->kva;
	/* 파일 페이지에 파일 정보 입력 */
	page->file.file = lm->file;
	page->file.addr = lm->file;
	page->file.upage = lm->upage;
	page->file.page_length = lm->page_read_bytes;
	page->file.total_length = lm->length;
	//off통과
	page->file.offset = lm->offset;
	if (kpage == NULL)
		return false;
	file_read(file,page->frame->kva, page_read_bytes);
	free(aux);
	return true;
}