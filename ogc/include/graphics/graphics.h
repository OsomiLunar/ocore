#pragma once
#include <algorithm>
#include "types/vector.h"
#include "utils/timer.h"
#include "graphics/generic.h"
#include "memory/blockallocator.h"
#include "types/bitset.h"
#include "template/enum.h"

namespace oi {
	
	namespace wc {
		class Window;
	}
	
	namespace gc {

		class RenderTarget;
		class TextureFormat;

		class GraphicsObject;
		class GraphicsResource;
		class TextureObject;

		enum class TextureFormatStorage;
		class TextureLoadFormat;

		UEnum(GraphicsFeature, Raytracing = 0, VR = 1);

		struct GraphicsExt;

		class Graphics {

			friend class GraphicsObject;
			
		public:

			static constexpr u32 maxId = 0xFFFFFF;
			
			Graphics(u32 heapSize) : heapSize(heapSize), allocator(heapSize), features(false), idAllocator(false) { idAllocator[0] = true; }
			~Graphics();
			
			void init(oi::wc::Window *w);
			
			void initSurface(oi::wc::Window *w);							//Inits surface & backbuffer
			void destroySurface();											//Destroys surface & backBuffer
			
			void begin();
			void end();
			void finish();

			template<typename T2>
			typename T2::ResourceType *create(String name, T2 info) {
				return init<typename T2::ResourceType, T2>(name, info);
			}

			GraphicsExt &getExtension();

			static bool isDepthFormat(TextureFormat format);
			static bool hasStencil(TextureFormat format);
			static u32 getChannelSize(TextureFormat format);						//Returns size of one channel in bytes
			static u32 getChannels(TextureFormat format);							//Returns number of channels
			static u32 getFormatSize(TextureFormat format);							//Returns size of pixel
			static TextureFormatStorage getFormatStorage(TextureFormat format);		//The type of a texture (float, uint, int)
			static bool isCompatible(TextureFormat a, TextureFormat b);				//Textures are compatible if they match channels and format storage
			static TextureLoadFormat getLoadFormat(TextureFormat format);

			static Vec4d convertColor(Vec4d color, TextureFormat format);			//Convert color to the correct params

			RenderTarget *getBackBuffer();
			u32 getBuffering();
			void printObjects();

			bool supports(GraphicsFeature feature);

			bool contains(GraphicsObject *go) const;
			bool destroyObject(GraphicsObject *go);
			void use(GraphicsObject *go);

			GraphicsObject *get(u32 id);
			bool contains(u32 id);
			void use(u32 id);
			void destroy(u32 id);

			template<typename T>
			bool destroy(T *&t);

			template<typename T>
			[[nodiscard]] std::vector<GraphicsObject*> get();

			template<typename T, typename T2>
			void alloc(T2 *&t2);

			template<typename T, typename T2>
			void dealloc(T2 *&t2);

		protected:

			template<typename T>
			void add(T *t);

			template<typename T, typename TInfo>
			T *init(String name, TInfo info);

			bool remove(GraphicsObject *go);

			void setupSurface(wc::Window *w);

		private:
			
			bool initialized = false;
			u32 buffering = 0, heapSize = 0;

			Timer renderTimer;

			RenderTarget *backBuffer = nullptr;

			oi::BlockAllocator allocator;
			oi::StaticBitset<maxId + 1> idAllocator;
			GraphicsExt *ext;

			std::unordered_map<size_t, std::vector<GraphicsObject*>> objects;
			std::unordered_map<u32, GraphicsObject*> objectsById;

			StaticBitset<GraphicsFeature::length> features;
			
		};
		

		template<typename T>
		void Graphics::add(T *t) {

			static_assert(std::is_base_of<GraphicsObject, T>::value, "Graphics::add is only available to GraphicsObjects");

			size_t id = typeid(T).hash_code();

			std::vector<GraphicsObject*> &o = objects[id];
			auto it = std::find(o.begin(), o.end(), (GraphicsObject*) t);

			if (it != o.end()) Log::warn("Graphics::add called on an already existing object");
			else {
				o.push_back(t);
				objectsById[t->getId()] = (GraphicsObject*) t;
			}

		}

		template<typename T>
		std::vector<GraphicsObject*> Graphics::get() {

			static_assert(std::is_base_of<GraphicsObject, T>::value, "Graphics::get is only available to GraphicsObjects");

			size_t id = typeid(T).hash_code();

			auto it = objects.find(id);
			if (it == objects.end()) return {};

			return it->second;
		}

		template<typename T, typename TInfo>
		T *Graphics::init(String name, TInfo info) {

			T *t = allocator.alloc<T, TInfo>(info);
			t->g = this;

			u32 id = maxId + 1;

			for(u32 i = 0; i < maxId; ++i)
				if (!idAllocator[i]) {
					id = i;
					idAllocator[i] = true;
					break;
				}

			if(id == maxId + 1)
				Log::throwError<Graphics, 0x2>("Couldn't init GraphicsObject; couldn't find a valid id");

			t->id = id;
			t->name = name;
			t->template setHash<T>();

			if (!t->init())
				return (T*) Log::throwError<Graphics, 0x0>("Couldn't init GraphicsObject");

			add(t);
			return t;
		}

		template<typename T>
		bool Graphics::destroy(T *&go) {

			static_assert(std::is_base_of<GraphicsObject, T>::value, "Graphics::destroy<T> requires T to be a base of GraphicsObject");

			static_assert(!std::is_same<GraphicsObject, T>::value && !std::is_same<GraphicsResource, T>::value && !std::is_same<TextureObject, T>::value, "Graphics::destroy can't be used on virtual GraphicsObjects, GraphicsResources or TextureObjects");

			if (go == nullptr) return false;

			auto it = objects.find(typeid(T).hash_code());
			if (it == objects.end()) return false;

			auto &vec = it->second;

			auto itt = std::find(vec.begin(), vec.end(), go);
			if (itt == vec.end()) return false;

			if (--go->refCount <= 0) {
				vec.erase(itt);
				objectsById.erase(go->getId());
				idAllocator[go->getId()] = false;
				allocator.dealloc(go);
				go = nullptr;
			}

			return true;
		}

		template<typename T, typename T2>
		void Graphics::alloc(T2 *&t2) {

			static_assert(std::is_same<typename T2::BaseType, T>::value, "Can't allocate if the extended type isn't part of the allocated type");

			t2 = allocator.alloc<T2>();

		}

		template<typename T, typename T2>
		void Graphics::dealloc(T2 *&t2) {

			static_assert(std::is_same<typename T2::BaseType, T>::value, "Can't allocate if the extended type isn't part of the allocated type");

			allocator.dealloc(t2);
			t2 = nullptr;

		}

	}
	
}