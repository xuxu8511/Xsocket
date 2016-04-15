#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "include.h"

namespace marco {

	class Buffer {
		public:
			Buffer(uint32_t init_size=2048);
			~Buffer();

			uint32_t Space();
			uint32_t Len();
			char *BeginWrite();
			void HasWrite(uint32_t len);
			char *BeginRead();
			void HasRead(uint32_t len);
			void Write(const char *data, uint32_t len);
			void Adjust();
			bool Grow();
			uint32_t Size();

		private:
			uint32_t read_index_;
			uint32_t write_index_;
			uint32_t size_;
			char *data_;
	};

}

#endif
