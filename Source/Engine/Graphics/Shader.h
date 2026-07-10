#pragma once

/*
//////////////////////////////////////////////////////////////

Main shader class used in the engine. In future can be adapted
for Vulkan Api, but now.. we use OpenGL 4.0

//////////////////////////////////////////////////////////////
*/
class ENGINE_API CShader
{
public:
	// default constructor
	CShader(void) { shID = 0; }
	// constructor
	CShader(const char* strPathVert, const char* strPathFrag);
//	CShader(const CShaderRequest& s);
	// destructor
	~CShader(void);

	// create shader
	void Create_t(const char* strPathVert, const char* strPathFrag);

	void CreateFromString(const char* VertShader, const char* FragShader);
	// use shader
	void use(void);
	// destroy shader
	void end(void);

	// set shaders value
	void shaderSet(const char* strName, const int iValue);
	void shaderSet(const char* strName, const float fValue);
	void shaderSet(const char* strName, const double dValue);
	// set shaders vectors
	void shaderSetVector(const char* strName, const float fx, const float fy);
	void shaderSetVector(const char* strName, const float fx, const float fy, const float fz);
	void shaderSetVector(const char* strName, const float fx, const float fy, const float fz, const float fw);
	// set shaders vectors
	void shaderSetVector(const char* strName, const glm::vec2& vector2);
	void shaderSetVector(const char* strName, const glm::vec3& vector3);
	void shaderSetVector(const char* strName, const glm::vec4& vector4);
	// set shaders matrix
	void shaderSetMatrix(const char* strName, const glm::mat3& matrix3);
	void shaderSetMatrix(const char* strName, const glm::mat4& matrix4);

	inline void shaderSet(uint& ID) { this->shID = ID; }

	//CShader &operator=(const CShader sha);

public:
	// program id
	unsigned int shID;
	// check compile errors
	void checkErrors(const unsigned int prog, const char* strError);
};


/* shader for put texture to screen (HUD, GUI, etc..) */
extern CShader shaTexture;
/* shader for blending screen and fill color */
extern CShader shaFillColor;
/* shader for rendering model */
extern CShader shaModel;
/* shader for rendering text font */
extern CShader shaText;
/* shader for rendering brush/room */
extern CShader shaPolygon;
/* shader for rendering skybox (this can be sets when world prepare to rendering) */
extern CShader shaSkyBox;