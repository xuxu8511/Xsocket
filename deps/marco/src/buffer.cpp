#include "buffer.h"
#include "include.h"

using namespace marco;

Buffer::Buffer(uint32_t init_size) :
	read_index_(0),
	write_index_(0),
	size_(init_size),
	data_(NULL)
{
//	LOG(INFO) << "buffer cons";
	assert(size_ > 0);
	data_ = (char *)malloc(size_);
}

Buffer::~Buffer() {
//	LOG(INFO) << "buffer des";
	::free(data_);
}

void Buffer::Write(const char *data, uint32_t len) {
	while(Space() < len) {
		if (!Grow()) {
      		assert(0);
		}
	}
	memcpy(BeginWrite(), data, len);
	HasWrite(len);
}

char* Buffer::BeginWrite() {
	return data_ + write_index_; 
}

void Buffer::HasWrite(uint32_t len) {
	write_index_ += len;
}

char* Buffer::BeginRead() {
	return data_ + read_index_;
}

void Buffer::HasRead(uint32_t len) {
	read_index_ += len;
}

uint32_t Buffer::Space() {
	return size_ - write_index_;
}

bool Buffer::Grow() {
	char *p = (char*)realloc(data_, 2 * size_);
	if (p == NULL) {
		return false;
	}
	data_ = p;
	size_ = 2 * size_;
	return true;
}

void Buffer::Adjust() {
	if(read_index_ > 0) {
		uint32_t len = Len();
		if(len > 0) {
			memmove(data_, data_ + read_index_, len);
			read_index_ = 0;
			write_index_ = len;
		}else{
			read_index_ = write_index_ = 0;
		}
	}
}

uint32_t Buffer::Len() {
	return write_index_ - read_index_; 
}

uint32_t Buffer::Size() {
	return size_;
}
