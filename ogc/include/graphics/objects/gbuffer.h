#pragma once

#include "graphicsobject.h"
#include "graphics/gl/generic.h"

namespace oi {

	namespace gc {

		DEnum(GBufferType, u32, UBO = 0, SSBO = 1, IBO = 2, VBO = 3, CBO = 4);

		class GBuffer;

		struct GBufferInfo {

			typedef GBuffer ResourceType;

			GBufferType type;
			Buffer buffer;

			bool hasData;

			//Empty gpu buffer
			GBufferInfo(GBufferType type, u32 size) : type(type), buffer(size), hasData(false) { buffer.clear(); }

			//Filled gpu buffer
			GBufferInfo(GBufferType type, Buffer buffer) : type(type), buffer(buffer), hasData(true) {}

		};

		class GBuffer : public GraphicsObject {

			friend class Graphics;
			friend class oi::BlockAllocator;

		public:

			GBufferType getType();

			u32 getSize();
			u8 *getAddress();

			Buffer getBuffer();

			GBufferExt &getExtension();
			const GBufferInfo &getInfo();

			bool copy(Buffer buf);
			void set(Buffer buf);		//copy(buf); flush();

			//Call flush to push changes to GPU
			void flush();

		protected:

			~GBuffer();
			GBuffer(GBufferInfo info);
			bool init();

		private:

			GBufferInfo info;
			GBufferExt ext;

		};

	}

}