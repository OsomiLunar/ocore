#pragma once

#include "gbuffer.h"
#include "shaderenums.h"
#include "graphicsresource.h"
#include <types/matrix.h>

namespace oi {

	namespace gc {

		enum class SBOFlag {
			Value = 0,
			Row = 1,
			Array = 2,
			RowArray = 3,
			DynamicArray = 6,
			DynamicRowArray = 7
		};

		struct ShaderBufferObject {

			ShaderBufferObject *parent;
			u32 offset, length;
			String name;
			TextureFormat format;
			SBOFlag flags;

			std::vector<ShaderBufferObject*> childs;
			std::vector<u32> arr;

			ShaderBufferObject(ShaderBufferObject *parent, u32 offset, u32 length, std::vector<u32> arr, String name, TextureFormat format, SBOFlag flags);
			ShaderBufferObject();

			void addChild(ShaderBufferObject *obj);
			ShaderBufferObject *find(String name);

		};

		struct ShaderBufferInfo {

			ShaderRegisterType type;
			u32 size;
			bool allocate;

			ShaderBufferObject self;
			std::vector<ShaderBufferObject> elements;

			ShaderBufferInfo(ShaderRegisterType type, u32 size, u32 elements, bool allocate = true);
			ShaderBufferInfo();

			void addRoot(ShaderBufferObject *obj);
			u32 getRoots();

			ShaderBufferObject &operator[](u32 i);
			ShaderBufferObject &getRoot(u32 i);

			u32 lookup(ShaderBufferObject *elem);

			ShaderBufferInfo &operator=(const ShaderBufferInfo &info);
			ShaderBufferInfo(const ShaderBufferInfo &info);

			void push(ShaderBufferObject obj, ShaderBufferObject &parent);

		private:

			void copy(const ShaderBufferInfo &info);

		};

		template<typename T> struct ShaderBufferCast { static bool check(u32 size, TextureFormat format) { return size == (u32) sizeof(T); } };
		template<> struct ShaderBufferCast<f32> { static bool check(u32 size, TextureFormat format) { return format == TextureFormat::R32f; } };
		template<> struct ShaderBufferCast<u32> { static bool check(u32 size, TextureFormat format) { return format == TextureFormat::R32u; } };
		template<> struct ShaderBufferCast<i32> { static bool check(u32 size, TextureFormat format) { return format == TextureFormat::R32i; } };

		template<typename T, u32 n> struct ShaderBufferCast<TVec<T, n>> { static bool check(u32 size, TextureFormat format) { 

			constexpr bool fl = std::is_floating_point<T>::value;
			constexpr bool uns = std::is_unsigned<T>::value;

			static_assert(fl || std::is_integral<T>::value, "Only supports vector of int/uint and float");

			return format.getValue() >= TextureFormat::RGBA32f && format.getValue() <= TextureFormat::R64i && 4 - (format.getValue() - TextureFormat::RGBA32f) % 4 == n && (fl ? format.getName().endsWith("f") : (uns ? format.getName().endsWith("u") : format.getName().endsWith("i")));
		} };

		template<typename T, u32 w, u32 h> struct ShaderBufferCast<TMatrix<T, w, h>> {
			static bool check(u32 size, TextureFormat format) {
				return ShaderBufferCast<TVec<T, w>>::check(size, format) && size == (u32) sizeof(TMatrix<T, w, h>);
			}
		};

		class ShaderBufferVar {

		public:

			ShaderBufferVar(ShaderBufferObject &obj, Buffer buf, bool available);

			template<typename T>
			T &cast() {

				if (!ShaderBufferCast<T>::check(buf.size(), obj.format))
					Log::throwError<ShaderBufferVar, 0x0>(String("Couldn't cast ShaderBufferVar with format ") + obj.format.getName() + " (" + obj.name + ")");

				return *(T*) buf.addr();
			}

		private:

			ShaderBufferObject &obj;
			Buffer buf;
			bool available;

		};

		class ShaderBuffer : public GraphicsResource {

			friend class Graphics;

		public:

			const ShaderBufferInfo getInfo();

			ShaderBufferVar get(String path);
			ShaderBufferVar get();

			u32 getElements();
			u32 getSize();

			GBuffer *getBuffer();

			//If info.allocate is false, you have to instantiate the buffer yourself
			//This can be a dynamic object buffer, light buffer, etc.
			ShaderBuffer *instantiate(u32 objects);	

			//If info.allocate is false, you can allocate a buffer yourself
			//This can be a dynamic object buffer, light buffer, etc.
			//The length of the GBuffer should match the ShaderBuffer's length
			void setBuffer(u32 objects, GBuffer *g);

			void open();					//Call this if you start writing/reading
			void copy(Buffer buf);			//Copy a buffer
			void close();					//Call this if you end writing

			void set(Buffer buf);			//Same as GBuffer::set; open(), copy(), close()

			template<typename T>
			T &get(String path);

			template<typename T>
			void set(String path, T t);

		protected:

			ShaderBuffer(ShaderBufferInfo info);
			~ShaderBuffer();

			bool init();

			void calculateArrayInfo(std::vector<u32> &off, std::vector<u32> &len, u32 stride, u32 &offset, u32 &count);
			void setObjectCount(u32 count);

		private:

			ShaderBufferInfo info;
			GBuffer *buffer = nullptr;
			Buffer current;

			bool isOpen = false;

		};


		template<typename T>
		T &ShaderBuffer::get(String path) {
			if (!isOpen) Log::throwError<ShaderBuffer, 0x0>("ShaderBuffer::set; buffer isn't open");
			return get(path).cast<T>();
		}

		template<typename T>
		void ShaderBuffer::set(String path, T t) {
			get<T>(path) = t;
		}

	}

}