#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <stddef.h>

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "gdiplus.lib")
#endif

#include <GL/glew.h>

#include <stdexcept>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <vector>
#include <map>

#include <GL/GLU.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/orthonormalize.hpp>


class GdiplusInitializer
{
public:
    GdiplusInitializer()
    {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartupOutput output;

        if (Gdiplus::GdiplusStartup(&m_token, &input, &output) != Gdiplus::Ok)
        {
            // Не удалось инициализировать GDI+
            throw std::runtime_error("Failed to initialize GDI+");
        }
    }

    ~GdiplusInitializer()
    {
        Gdiplus::GdiplusShutdown(m_token);
    }
private:
    ULONG_PTR m_token;
};


class GLApplication
{
public:
	void MainLoop();

protected:
	GLApplication(
		const char* title,
		int width = 0,
		int height = 0,
		bool needDepth = true,
		bool needStencil = false
	);
	virtual ~GLApplication(void);

	virtual void OnInit();
	virtual void OnDisplay() = 0;
	virtual void OnReshape(int width, int height);
	virtual void OnKeyboard(unsigned char key, int x, int y);
	virtual void OnKeyboardUp(unsigned char key, int x, int y);
	virtual void OnMouse(int button, int state, int x, int y);
	virtual void OnMotion(int x, int y);
	virtual void OnSpaceballMotion(int x, int y, int z);
	virtual void OnSpaceballRotation(int x, int y, int z);
	virtual void OnSpaceballButton(int button, int state);
	virtual void OnSpecial(int key, int x, int y);
	virtual void OnIdle();

	void PostRedisplay();

private:
	void InitEventHandlers();
	static void DisplayHandler();
	static void ReshapeHandler(int width, int height);
	static void KeyboardHandler(unsigned char key, int x, int y);
	static void KeyboardUpHandler(unsigned char key, int x, int y);
	static void MouseHandler(int button, int state, int x, int y);
	static void MotionHandler(int x, int y);
	static void IdleHandler();
	static void SpaceballMotionHandler(int x, int y, int z);
	static void SpaceballRotateHandler(int x, int y, int z);
	static void SpaceballButtonHandler(int button, int state);
	static void SpecialHandler(int key, int x, int y);
	static GLApplication* m_pApplication;
};


GLApplication* GLApplication::m_pApplication = nullptr;

GLApplication::GLApplication(
	const char* title,
	int width,
	int height,
	bool needDepth,
	bool needStencil)
{
	// Только один экземпляр приложения разрешено создать
	if (m_pApplication)
	{
		assert(!"Only one instance of the application is permitted");
		return;
	}
	m_pApplication = this;

	// Инициализируем библиотеку GLUT, подсовывая ей
	// фиктивные параметры командой строки
	int argc = 1;
	char const* argv[] = { "" };
	glutInit(&argc, const_cast<char**>(argv));

	// Задаем режим буфера кадра, запрошенный пользователем
	glutInitDisplayMode(
		GLUT_RGBA | GLUT_DOUBLE | (needDepth ? GLUT_DEPTH : 0) | (needStencil ? GLUT_STENCIL : 0));

	// Если пользователь указал размеры окна, сообщаем о них GLUT
	if (width > 0 && height > 0)
	{
		glutInitWindowSize(width, height);
	}

	// Создаем окно приложения
	glutCreateWindow(title);

	// Задаем обработчики событий
	InitEventHandlers();
}

void GLApplication::InitEventHandlers()
{
	// Инициализируем обработчики часто используемых событий
	glutDisplayFunc(&DisplayHandler);
	glutReshapeFunc(&ReshapeHandler);
	glutKeyboardFunc(&KeyboardHandler);
	glutMouseFunc(&MouseHandler);
	glutMotionFunc(&MotionHandler);
	glutIdleFunc(&IdleHandler);
	glutSpecialFunc(&SpecialHandler);

#if (GLUT_API_VERSION >= 2)
	glutSpaceballMotionFunc(&SpaceballMotionHandler);
	glutSpaceballRotateFunc(&SpaceballRotateHandler);
	glutSpaceballButtonFunc(&SpaceballButtonHandler);
#endif
	// При желании можно добавить обработчики остальных
	// событий, поддерживаемых glut
	glutKeyboardFunc(&KeyboardHandler);
#if defined(GLUT_KEY_REPEAT_OFF) || defined(FREEGLUT)
	glutKeyboardUpFunc(&KeyboardUpHandler);
#endif
}

void GLApplication::SpaceballMotionHandler(int x, int y, int z)
{
	m_pApplication->OnSpaceballMotion(x, y, z);
}

void GLApplication::SpaceballRotateHandler(int x, int y, int z)
{
	m_pApplication->OnSpaceballRotation(x, y, z);
}

void GLApplication::SpaceballButtonHandler(int button, int state)
{
	m_pApplication->OnSpaceballButton(button, state);
}

void GLApplication::MotionHandler(int x, int y)
{
	// Переадресовываем вызов обработчика события экземпляру класса приложения
	m_pApplication->OnMotion(x, y);
}

void GLApplication::MouseHandler(int button, int state, int x, int y)
{
	// Переадресовываем вызов обработчика события экземпляру класса приложения
	m_pApplication->OnMouse(button, state, x, y);
}

void GLApplication::DisplayHandler()
{
	// Переадресовываем вызов обработчика события экземпляру класса приложения
	m_pApplication->OnDisplay();
	// Обмениваем местами теневой и лицевой буферы кадра
	glutSwapBuffers();
}

void GLApplication::IdleHandler()
{
	Sleep(1);
	// ничего не делаем
	m_pApplication->OnIdle();
}

void GLApplication::ReshapeHandler(int width, int height)
{
	// Переадресовываем вызов обработчика события экземпляру класса приложения
	m_pApplication->OnReshape(width, height);
}

void GLApplication::KeyboardHandler(unsigned char key, int x, int y)
{
	// Переадресовываем вызов обработчика события экземпляру класса приложения
	m_pApplication->OnKeyboard(key, x, y);
}

void GLApplication::KeyboardUpHandler(unsigned char key, int x, int y)
{
	// Переадресовываем вызов обработчика события экземпляру класса приложения
	m_pApplication->OnKeyboardUp(key, x, y);
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnReshape(int width, int height)
{
	// Обработчик по умолчанию всего лишь задает
	// порт просмотра размером с клиентскую область окна
	glViewport(0, 0, width, height);
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnKeyboard(unsigned char /*key*/, int /*x*/, int /*y*/)
{
}

void GLApplication::OnKeyboardUp(unsigned char /*key*/, int /*x*/, int /*y*/)
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnMouse(int /*button*/, int /*state*/, int /*x*/, int /*y*/)
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnMotion(int /*x*/, int /*y*/)
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnIdle()
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnSpaceballMotion(int /*x*/, int /*y*/, int /*z*/)
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnSpaceballRotation(int /*x*/, int /*y*/, int /*z*/)
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnSpaceballButton(int /*button*/, int /*state*/)
{
}

void GLApplication::SpecialHandler(int key, int x, int y)
{
	m_pApplication->OnSpecial(key, x, y);
}

void GLApplication::OnSpecial(int /*key*/, int /*x*/, int /*y*/)
{
}

// Метод может быть перегружен в классе-наследнике
void GLApplication::OnInit()
{
}

void GLApplication::PostRedisplay()
{
	// Инициируем перерисовку калпа
	glutPostRedisplay();
}

void GLApplication::MainLoop(void)
{
	OnInit();
	glutMainLoop();
}

GLApplication::~GLApplication()
{
}




class BaseTexture
{
public:
	void Create()
	{
		assert(!m_texture);
		glGenTextures(1, &m_texture);
	}

	void Delete()
	{
		assert(m_texture);
		glDeleteTextures(1, &m_texture);
		m_texture = 0;
	}

	GLuint Detach()
	{
		GLuint texture = m_texture;
		m_texture = 0;
		return texture;
	}

	operator GLuint() const
	{
		return m_texture;
	}

	void BindTo(GLenum target) const
	{
		assert(m_texture != 0);
		glBindTexture(target, m_texture);
	}

protected:
	BaseTexture(GLuint texture)
		:m_texture(texture)
	{
	}

private:
	GLuint m_texture;

private:
	BaseTexture(BaseTexture const&);
	BaseTexture& operator=(BaseTexture const&);
};



template <bool t_managed, class TBase>
class TextureImpl : public TBase
{
public:
	TextureImpl(GLuint texture = 0)
		:TBase(texture)
	{
	}

	~TextureImpl()
	{
		bool m = t_managed;
		if (m && (*this != 0))
		{
			TBase::Delete();
		}
	}

	void Attach(GLuint texture)
	{
		if (t_managed && (*this != 0) && (texture != *this))
		{
			TBase::Delete();
		}
		TBase::SetTexture(texture);
	}
};



class Texture2DImpl : public BaseTexture
{
public:
	Texture2DImpl(GLuint texture = 0)
		:BaseTexture(texture)
	{
	}

	void Bind() const
	{
		BindTo(GL_TEXTURE_2D);
	}

	static void TexImage(
		GLint level,
		GLint internalFormat,
		GLsizei width, GLsizei height,
		GLint border,
		GLenum format, GLenum type,
		const GLvoid* pixels)
	{
		glTexImage2D(GL_TEXTURE_2D, level, internalFormat,
			width, height, border, format, type, pixels);
	}
};


class Texture1DImpl : public BaseTexture
{
public:
	Texture1DImpl(GLuint texture = 0)
		:BaseTexture(texture)
	{
	}

	virtual void Bind() const
	{
		BindTo(GL_TEXTURE_1D);
	}

	static void TexImage(
		GLint level,
		GLint internalFormat,
		GLsizei width,
		GLint border,
		GLenum format, GLenum type,
		const GLvoid* pixels)
	{
		glTexImage1D(GL_TEXTURE_1D, level, internalFormat,
			width, border, format, type, pixels);
	}
};

typedef TextureImpl<true, BaseTexture> Texture;
typedef TextureImpl<false, BaseTexture> TextureHandle;

typedef TextureImpl<true, Texture2DImpl> Texture2D;
typedef TextureImpl<false, Texture2DImpl> Texture2DHandle;

typedef TextureImpl<true, Texture1DImpl> Texture1D;
typedef TextureImpl<false, Texture1DImpl> Texture1DHandle;



class TextureLoader
{
public:
	TextureLoader() :
		m_buildMipmaps(true),
		m_wrapS(GL_REPEAT),
		m_wrapT(GL_REPEAT),
		m_minFilter(GL_LINEAR_MIPMAP_LINEAR),
		m_magFilter(GL_LINEAR)
	{
	}

	GLuint LoadTexture2D(std::wstring const& fileName, GLuint textureName = 0, GLint level = 0) const
	{
		Gdiplus::Bitmap bmp(fileName.c_str());

		if (bmp.GetLastStatus() != Gdiplus::Ok)
		{
			throw std::runtime_error("Error loading texture file");
		}

		bmp.RotateFlip(Gdiplus::RotateNoneFlipY);
		Gdiplus::PixelFormat pixelFormat = 0;
		GLint textureFormat = 0;
		GLint colorComponents = 0;
		GLint internalFormat = 0;

		if ((bmp.GetPixelFormat() & PixelFormatAlpha) != 0)
		{
			colorComponents = 4;
			pixelFormat = PixelFormat32bppARGB;
			textureFormat = GL_BGRA_EXT;
			internalFormat = GL_RGBA;
		}
		else
		{
			colorComponents = 3;
			textureFormat = GL_BGR_EXT;
			pixelFormat = PixelFormat24bppRGB;
			internalFormat = GL_RGB;
		}
		Gdiplus::Rect lockRect(0, 0, bmp.GetWidth(), bmp.GetHeight());

		Gdiplus::BitmapData bitmapData{};
		bitmapData.Scan0 = NULL;
		bmp.LockBits(&lockRect, Gdiplus::ImageLockModeRead, pixelFormat, &bitmapData);

		Texture2DHandle texture(textureName);
		if (textureName == 0)
		{
			texture.Create();
		}


		texture.Bind();

		if (m_buildMipmaps)
		{
			gluBuild2DMipmaps(
				GL_TEXTURE_2D,
				colorComponents,
				bitmapData.Width, bitmapData.Height,
				textureFormat, GL_UNSIGNED_BYTE,
				bitmapData.Scan0);
		}
		else
		{
			texture.TexImage(
				level,
				internalFormat,
				bitmapData.Width, bitmapData.Height,
				0,
				textureFormat,
				GL_UNSIGNED_BYTE,
				bitmapData.Scan0);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapT);


		bmp.UnlockBits(&bitmapData);

		return texture;
	}

	void SetBuildMipmaps(bool buildMipmaps)
	{
		m_buildMipmaps = buildMipmaps;
	}

	void SetMinFilter(GLenum minFilter)
	{
		m_minFilter = m_minFilter;
	}

	void SetMagFilter(GLenum magFilter)
	{
		m_magFilter = m_magFilter;
	}

	void SetWrapMode(GLenum wrapS, GLenum wrapT)
	{
		m_wrapS = wrapS;
		m_wrapT = wrapT;
	}

private:
	bool m_buildMipmaps;
	GLenum m_wrapS;
	GLenum m_wrapT;
	GLenum m_minFilter;
	GLenum m_magFilter;
};
