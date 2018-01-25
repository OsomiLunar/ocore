#pragma once

#include <Graphics/Material/Shader.h>
#include "ShaderInput.h"

namespace oi {

	namespace gc {

		struct OpenGLShaderStage : public ShaderStageData {
			GLuint id;

			OpenGLShaderStage(GLuint _id): id(_id) {}
		};

		class OpenGLShader : public Shader {

		public:

			OpenGLShader();
			~OpenGLShader();
			bool init(ShaderInfo info) override;
			void bind() override;
			void unbind() override;
			bool isValid() override;

		protected:

			void cleanup(ShaderStageData *stage) override;

			OString getExtension(ShaderStage stage) override;

			ShaderStageData *compile(ShaderInfo &si, ShaderStage which) override;
			bool link(ShaderStageData **data, u32 count) override;
			bool logError(GLuint handle);

			bool genReflectionData() override;

			GLenum pickFromStage(ShaderStage which);

		private:

			GLuint shaderId;
			std::vector<ShaderInput> uniforms, attributes;

		};

	}
}