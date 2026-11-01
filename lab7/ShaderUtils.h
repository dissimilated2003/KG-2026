#pragma once


#include "Utils.h"

#include <sstream>
#include <fstream>


class BufferObjectBase
{
public:
	// Генерируем имя для буферного объекта
	void Create()
	{
		if (m_bufferId == 0)
		{
			glGenBuffers(1, &m_bufferId);
		}
	}

	// Возвращаем идентификатор буферного объекта
	operator GLuint() const
	{
		return m_bufferId;
	}

	// "Отсоединяем" буферный объект от текущего экземпляра класса
	GLuint Detach()
	{
		GLuint bufferId = m_bufferId;
		m_bufferId = 0;
		return bufferId;
	}

	// Удаляем буферный объект
	void Delete()
	{
		if (m_bufferId != 0)
		{
			glDeleteBuffers(1, &m_bufferId);
			m_bufferId = 0;
		}
	}
protected:
	BufferObjectBase(GLuint bufferId)
		: m_bufferId(bufferId)
	{
	}

	void BindTo(GLenum target) const
	{
		glBindBuffer(target, m_bufferId);
	}

	void SetBuffer(GLuint bufferId)
	{
		m_bufferId = bufferId;
	}

private:
	GLuint m_bufferId;

	BufferObjectBase(BufferObjectBase const&);
	BufferObjectBase& operator=(BufferObjectBase const&);
};


template <bool t_managed, GLenum target>
class BufferObjectImpl : public BufferObjectBase
{
public:
	BufferObjectImpl<t_managed, target>(GLuint bufferId = 0)
		: BufferObjectBase(bufferId)
	{
	}

	~BufferObjectImpl()
	{
		if (t_managed && (*this != 0))
		{
			Delete();
		}
	}

	void Bind() const
	{
		BindTo(target);
	}

	void Attach(GLuint bufferId)
	{
		if (t_managed && (*this != 0) && (bufferId != *this))
		{
			Delete();
		}
		SetBuffer(bufferId);
	}

	static GLvoid* MapBuffer(GLenum access)
	{
		return glMapBuffer(target, access);
	}

	static GLboolean UnmapBuffer()
	{
		return glUnmapBuffer(target);
	}

	static void BufferData(GLsizeiptr size, GLvoid const* data, GLenum usage)
	{
		glBufferData(target, size, data, usage);
	}

	static void BufferSubData(GLintptr offset, GLsizeiptr size, const GLvoid* data)
	{
		glBufferSubData(target, offset, size, data);
	}

	BufferObjectImpl<t_managed, target>& operator=(GLuint bufferId)
	{
		Attach(bufferId);
		return *this;
	}
};

typedef BufferObjectImpl<true, GL_ARRAY_BUFFER> ArrayBuffer;
typedef BufferObjectImpl<false, GL_ARRAY_BUFFER> ArrayBufferHandle;
typedef BufferObjectImpl<true, GL_ELEMENT_ARRAY_BUFFER> ElementArrayBuffer;
typedef BufferObjectImpl<false, GL_ELEMENT_ARRAY_BUFFER> ElementArrayBufferHandle;


template <GLenum target>
class BufferImpl
{
public:
	BufferImpl(bool useVBO = true)
		:m_hasData(false)
		, m_useVBO(useVBO)
		, m_created(false)
	{
	}

	void Create()
	{
		if (!m_created)
		{
			m_useVBO = m_useVBO && (GLEW_ARB_vertex_buffer_object == GL_TRUE);
			if (m_useVBO && (m_buffer == 0))
			{
				m_buffer.Create();
			}
			m_created = true;
		}
	}

	void Bind() const
	{
		if (!m_created)
		{
			throw std::logic_error("The buffer has not been created");
		}

		if (m_useVBO)
		{
			m_buffer.Bind();
		}
		else if (GLEW_ARB_vertex_buffer_object)
		{
			glBindBuffer(target, 0);
		}

		UpdateCurrentBufferPointer(false, this);
	}

	static void Unbind()
	{
		if (GLEW_ARB_vertex_buffer_object)
		{
			glBindBuffer(target, 0);
		}

		UpdateCurrentBufferPointer(false, NULL);
	}

	void BufferData(GLsizeiptr size, GLvoid const* data, GLenum usage)
	{
		UpdateCurrentBufferPointer(true, this);

		if (m_useVBO)
		{
			m_buffer.BufferData(size, data, usage);
		}
		else
		{
			if (data != NULL)
			{
				GLubyte const* pDataStart = reinterpret_cast<GLubyte const*>(data);
				GLubyte const* pDataEnd = pDataStart + size;
				m_data.assign(pDataStart, pDataEnd);
			}
			else
			{
				m_data.clear();
			}
		}

		m_hasData = (data != NULL);
	}

	GLvoid const* GetBufferPointer()const
	{
		if (!m_hasData)
		{
			throw std::logic_error("The buffer contains no data");
		}

		UpdateCurrentBufferPointer(true, this);

		if (m_useVBO)
		{
			return NULL;
		}
		else
		{
			return &m_data[0];
		}
	}

	~BufferImpl()
	{
		UpdateCurrentBufferPointer(false, NULL);
	}

private:
	static void UpdateCurrentBufferPointer(bool callBind, BufferImpl const* thisPtr)
	{
		static const BufferImpl* pCurrentBuffer = NULL;
		if (pCurrentBuffer != thisPtr)
		{
			pCurrentBuffer = thisPtr;
			if (callBind && (thisPtr != NULL))
			{
				thisPtr->Bind();
			}
		}
	}


private:
	BufferImpl(BufferImpl const&);
	BufferImpl& operator=(BufferImpl const&);

	BufferObjectImpl<true, target> m_buffer;
	std::vector<BYTE>	m_data;

	bool m_hasData;
	bool m_useVBO;
	bool m_created;
};

typedef BufferImpl<GL_ARRAY_BUFFER> VertexBuffer;
typedef BufferImpl<GL_ELEMENT_ARRAY_BUFFER> IndexBuffer;



/*
Класс "Базовый программный объект OpenGL"
*/
class ProgramBase
{
protected:
    ProgramBase(GLuint program = 0)
        :m_program(program)
    {
    }

    // Объявляем деструктор защищенным, чтобы нельзя было удалить
    // экземпляр класса через CProgramBase
    ~ProgramBase()
    {
    }

public:
    // Получаем целочисленный идентификатор программного объекта
    operator GLuint()const
    {
        return m_program;
    }

    // Получаем целочисленный идентификатор программного объекта
    GLuint Get()const
    {
        return m_program;
    }

    // Удаляем программный объект
    void Delete()
    {
        if (m_program != 0)
        {
            glDeleteProgram(m_program);
            m_program = 0;
        }
    }

    // Присоединям к объекту класса дескриптор программного объекта,
    // возвращая ранее существующий
    GLuint Attach(GLuint program)
    {
        GLuint tmp = m_program;
        m_program = program;
        return tmp;
    }

    /*
    Присоединяем к программе шейдер с указанным идентификатором
    */
    void AttachShader(GLuint shader)
    {
        assert(m_program != 0);
        glAttachShader(m_program, shader);
    }

    // Отсоединяем шейдер с указанным идентификатором
    void DetachShader(GLuint shader)
    {
        assert(m_program != 0);
        glDetachShader(m_program, shader);
    }

    // Выполняем компоновку программы
    void Link()const
    {
        assert(m_program);
        glLinkProgram(m_program);
    }

    // Проверяем, возможна ли корректная работа шейдерной программы
    // с текущим состоянием OpenGL
    void Validate()const
    {
        assert(m_program);
        glValidateProgram(m_program);
    }

    // Получаем информацию о программе в текстовом виде
    void GetInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog)const
    {
        assert(m_program != 0);
        glGetProgramInfoLog(m_program, bufSize, length, infoLog);
    }

    // Получаем информацию о программе в текстовом виде
    // (упрощенный вариант)
    std::string GetInfoLog()const
    {
        GLint length = GetParameter(GL_INFO_LOG_LENGTH);
        if (length > 0)
        {
            std::vector<char> buffer(length);
            GetInfoLog(length, &length, &buffer[0]);
            std::string log(&buffer[0], length - 1);
            return log;
        }
        else
        {
            return std::string();
        }
    }

    // Задаем параметр программного объекта
    void GetParameter(GLenum pname, GLint* param)const
    {
        assert(m_program != 0);
        glGetProgramiv(m_program, pname, param);
    }

    // Получаем параметр программного объекта
    GLint GetParameter(GLenum pname)const
    {
        GLint value = 0;
        GetParameter(pname, &value);
        return value;
    }

    // Получаем расположение uniform-переменной с
    // указанным именем
    GLint GetUniformLocation(const GLchar* name)const
    {
        assert(m_program);
        return glGetUniformLocation(m_program, name);
    }

    // Получаем расположение атрибутивной переменной
    // с заданным именем.
    GLint GetAttribLocation(const GLchar* name)const
    {
        assert(m_program);
        return glGetAttribLocation(m_program, name);
    }

    // Возвращаем количество активных uniform-переменных
    GLuint GetActiveUniforms()const
    {
        return GetParameter(GL_ACTIVE_UNIFORMS);
    }

    // Возвращаем количество активных attribute-переменных
    GLuint GetActiveAttributes()const
    {
        return GetParameter(GL_ACTIVE_ATTRIBUTES);
    }

    // Возвращаем максимальную длину для хранения имени
    // uniform-переменной программы
    GLuint GetActiveUniformMaxLength()const
    {
        return GetParameter(GL_ACTIVE_UNIFORM_MAX_LENGTH);
    }

    // Получаем информацию об имени, типе и размере uniform-переменной
    void GetActiveUniform(
        GLuint index, GLsizei maxLength,
        GLsizei* length, GLint* size,
        GLenum* type, GLchar* name)const
    {
        assert(m_program);
        assert(index < GetActiveUniforms());
        glGetActiveUniform(m_program, index, maxLength, length, size, type, name);
    }

    // Получаем информацию об имени, типе и размере uniform-переменной
    // более простым способом
    std::string GetActiveUniform(GLuint index, GLint* size, GLenum* type)const
    {
        // Узнаем размер буфера для хранения имени
        GLuint bufferLength = GetActiveUniformMaxLength();
        if (!bufferLength)
        {
            // Нет активных uniform переменных
            return std::string();
        }
        std::vector<char> buffer(bufferLength);
        GLsizei nameLength = 0;

        // Получаем имя, тип и размер переменной
        GetActiveUniform(index, bufferLength, &nameLength, size, type, &buffer[0]);

        // Переводим их в строковое представление
        return std::string(&buffer[0], &buffer[nameLength]);
    }

    // Возвращаем максимальную длину для хранения имени
    // attribute-переменной программы
    GLuint GetActiveAttributeMaxLength()const
    {
        return GetParameter(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH);
    }

    // Получаем информацию об имени, типе, длине и размере активной
    // attribute-переменной
    void GetActiveAttrib(
        GLuint index, GLsizei maxLength,
        GLsizei* length, GLint* size,
        GLenum* type, GLchar* name)const
    {
        assert(m_program);
        assert(index < GetActiveAttributes());
        glGetActiveAttrib(m_program, index, maxLength, length, size, type, name);
    }

    // Получаем информацию об имени, размере и типе активной
    // attribute-переменной более простым способом
    std::string GetActiveAttrib(GLuint index, GLint* size, GLenum* type)const
    {
        GLuint bufferLength = GetActiveAttributeMaxLength();
        if (!bufferLength)
        {
            // Нет активных attribute-переменных
            return std::string();
        }
        std::vector<char> buffer(bufferLength);
        GLsizei nameLength = 0;

        // Получаем имя, тип и размер переменной
        GetActiveAttrib(
            index, bufferLength,
            &nameLength, size,
            type, &buffer[0]
        );

        // Переводим их в строковое представление
        return std::string(&buffer[0], &buffer[nameLength]);
    }

    

private:
    ProgramBase(ProgramBase const&);
    ProgramBase& operator=(ProgramBase const&);
    GLuint m_program;
};


/*
Управляемая либо неуправляемая реализация программного объекта
*/
template<bool t_managed>
class ProgramImpl : public ProgramBase
{
public:
    ProgramImpl(GLuint program = 0)
        : ProgramBase(program)
    {
    }

    // Создаем программный объект и возвращаем его идентификатор
    GLuint Create()
    {
        if (t_managed && (Get() != 0))
        {
            Delete();
        }
        GLuint program = glCreateProgram();
        Attach(program);
        return program;
    }

    // Выполняем замену текущего программного объекта
    ProgramImpl& operator=(GLuint program)
    {
        if (t_managed && (Get() != 0) && (Get() != program))
        {
            Delete();
        }
        Attach(program);
        return *this;
    }

    // Деструктор программного объекта
    ~ProgramImpl()
    {
        // удаляем объект OpenGL (если он управляется классом)
        if (t_managed && Get() != 0)
        {
            Delete();
        }
    }
};

// Тип "Программа
// (с автоматическим управлением времем жизн
// программного объекта OpenGL
typedef ProgramImpl<true> Program;

// Тип "Дескриптор программы"
// (без автоматического управления временем жизни
// программного объекта OpenGL)
typedef ProgramImpl<false> ProgramHandle;



class ProgramInfo
{
public:
    ProgramInfo(GLuint program)
        : m_program(program)
    {
    }

    static std::string TypeToString(GLenum type)
    {
        struct TypeMap
        {
            GLenum type;
            const char* typeName;
        };

        TypeMap const typeMap[] =
        {
            {GL_FLOAT, "float"},
            {GL_FLOAT_VEC2, "vec2"},
            {GL_FLOAT_VEC3, "vec3"},
            {GL_FLOAT_VEC4, "vec4"},
            {GL_INT, "int"},
            {GL_INT_VEC2, "ivec2"},
            {GL_INT_VEC3, "ivec3"},
            {GL_INT_VEC4, "ivec4"},
            {GL_BOOL, "bool"},
            {GL_BOOL_VEC2, "bvec2"},
            {GL_BOOL_VEC3, "bvec3"},
            {GL_BOOL_VEC4, "bvec4"},
            {GL_FLOAT_MAT2, "mat2"},
            {GL_FLOAT_MAT3, "mat3"},
            {GL_FLOAT_MAT4, "mat4"},
            {GL_FLOAT_MAT2x3, "mat2x3"},
            {GL_FLOAT_MAT2x4, "mat2x4"},
            {GL_FLOAT_MAT3x2, "mat3x2"},
            {GL_FLOAT_MAT3x4, "mat3x4"},
            {GL_FLOAT_MAT4x2, "mat4x2"},
            {GL_FLOAT_MAT4x3, "mat4x3"},
            {GL_SAMPLER_1D, "sampler1D"},
            {GL_SAMPLER_2D, "sampler2D"},
            {GL_SAMPLER_3D, "sampler3D"},
            {GL_SAMPLER_CUBE, "samplerCube"},
            {GL_SAMPLER_1D_SHADOW, "sampler1DShadow"},
            {GL_SAMPLER_2D_SHADOW, "sampler2DShadow"}
        };
        static const unsigned TYPE_MAP_ITEMS_COUNT = sizeof(typeMap) / sizeof(*typeMap);
        for (unsigned int i = 0; i < TYPE_MAP_ITEMS_COUNT; ++i)
        {
            if (type == typeMap[i].type)
            {
                return typeMap[i].typeName;
            }
        }
        throw std::invalid_argument("Unknown variable type");
    }

    GLuint GetActiveUniforms()const
    {
        return m_program.GetActiveUniforms();
    }

    void PrintUniformInfo(GLuint index, std::ostream& stream) const
    {
        GLint uniformSize = 0;
        GLenum uniformType = 0;

        std::string name = m_program.GetActiveUniform(index, &uniformSize, &uniformType);
        std::string type = TypeToString(uniformType);

        stream << type << " " << " " << name;

        if (uniformSize != 1)
        {
            stream << "[" << uniformSize << "]";
        }

        if (name.length() > 3 && name.substr(0, 3) != "gl_")
        {
            GLint location =
                m_program.GetUniformLocation(name.c_str());
            stream << " at " << location;
        }
    }

    GLuint GetActiveAttributes() const
    {
        return m_program.GetActiveAttributes();
    }

    void PrintAttributeInfo(GLuint index, std::ostream& stream) const
    {
        GLint uniformSize = 0;
        GLenum uniformType = 0;
        std::string name = m_program.GetActiveAttrib(index, &uniformSize, &uniformType);
        std::string type = TypeToString(uniformType);
        stream << type << " " << " " << name;

        if (uniformSize != 1)
        {
            stream << "[" << uniformSize << "]";
        }

        if (name.length() > 3 && name.substr(0, 3) != "gl_")
        {
            GLint location = m_program.GetAttribLocation(name.c_str());
            stream << " at " << location;
        }
    }

    void Print(std::ostream& stream) const
    {
        stream << "Program id: " << m_program.Get() << "\n";
        if (m_program.GetActiveUniformMaxLength())
        {
            GLuint const activeUniforms = GetActiveUniforms();
            stream << "  Active uniforms: " << activeUniforms << "\n";
            for (GLuint uniform = 0; uniform < activeUniforms; ++uniform)
            {
                stream << "    ";
                PrintUniformInfo(uniform, stream);
                stream << "\n";
            }
        }

        if (m_program.GetActiveAttributeMaxLength() != 0)
        {
            GLuint const activeAttributes = GetActiveAttributes();
            stream << "  Active attributes: " << activeAttributes << "\n";
            for (GLuint attribute = 0; attribute < activeAttributes; ++attribute)
            {
                stream << "    ";
                PrintAttributeInfo(attribute, stream);
                stream << "\n";
            }
        }
    }


private:
    ProgramInfo(ProgramInfo const&) = delete;
    ProgramInfo& operator=(ProgramInfo const&) = delete;

    ProgramHandle m_program;
};


//Класс «Компоновщик шейдерных программ»
class ProgramLinker
{
public:
    ProgramLinker(void) {}
    ~ProgramLinker(void) {}

    void LinkProgram(GLuint program)
    {
        // Компонуем программу и добавляем ее в массив
        // компонуемых программ
        ProgramHandle prog(program);
        prog.Link();
        m_programs.push_back(program);
    }

    // Проверяем статус скомпонованности шейдерных программ
    void CheckStatus()
    {
        std::stringstream strm;
        bool hasErrors = false;
        for (size_t i = 0; i < m_programs.size(); ++i)
        {
            ProgramHandle program(m_programs[i]);
            if (program.GetParameter(GL_LINK_STATUS) != GL_TRUE)
            {
                hasErrors = true;
                strm << "Program " << program << " linkage failed: " <<
                    program.GetInfoLog() << "\n";
            }
        }

        m_programs.clear();
        if (hasErrors)
        {
            throw std::runtime_error(strm.str());
        }
    }

private:
    std::vector<GLuint> m_programs;
};


/*
Базовый класс для работы с шейдерный объектом OpenGL
*/
class ShaderBase
{
    // Защищенные конструктор и деструктор сделают
    // возможным использование данного класса только
    // для его потомков
protected:
    ShaderBase(GLuint shader = 0)
        :m_shader(shader)
    {
    }

    // Объявляем деструктор защищенным, т.к. в противном
    // случае компилятор сделал бы его публичным
    // Объявлять деструктор виртуальным нет необходимости
    // т.к. деструктор защищен от вызова извне, а значит
    // удалить CShaderBase напрямую не получится (только удалив
    // его наследника)
    ~ShaderBase()
    {
        // Данный деструктор не делает ничего
        // Освобождением ресурсов будет заниматься
        // класс-потомок
    }
private:
    // Делаем недоступными конструктор копирования
    // и оператор присваивания
    ShaderBase(ShaderBase const&);
    ShaderBase& operator=(ShaderBase const&);

    GLuint m_shader;

public:
    // Задаем исходный код шейдера
    void SetSource(GLsizei count, const GLchar** strings, const GLint* lengths)
    {
        assert(m_shader != 0);
        glShaderSource(m_shader, count, strings, lengths);
    }

    // Задаем исходный код шейдера (в одной строке)
    void SetSource(const GLchar* source, GLint length)
    {
        const GLchar** ppSource = &source;
        SetSource(1, ppSource, &length);
    }

    // Задаем исходный код шейдера (одна ASCIIZ-строка)
    void SetSource(const GLchar* source)
    {
        const GLchar** ppSource = &source;
        SetSource(1, ppSource, NULL);
    }

    // Получаем значение некоторого параметра шейдера
    void GetParameter(GLenum pname, GLint* param)const
    {
        assert(m_shader != 0);
        glGetShaderiv(m_shader, pname, param);
    }

    // Упрощенный вариант предыдущего метода
    GLint GetParameter(GLenum pname)const
    {
        GLint value;
        GetParameter(pname, &value);
        return value;
    }

    // Получаем информационный лог от шейдера
    void GetInfoLog(GLsizei bufSize, GLsizei* length, GLchar* infoLog)const
    {
        assert(m_shader != 0);
        glGetShaderInfoLog(m_shader, bufSize, length, infoLog);
    }

    // Получаем информационный лог от шейдера
    // (упрощенный вариант)
    std::string GetInfoLog()const
    {
        GLint length = GetParameter(GL_INFO_LOG_LENGTH);
        if (length > 0)
        {
            std::vector<char> buffer(length);
            GetInfoLog(length, &length, &buffer[0]);
            std::string log(&buffer[0]);
            return log;
        }
        else
        {
            return std::string();
        }
    }

    // Получаем идентификатор шейдерного объекта
    operator GLuint()const
    {
        return m_shader;
    }

    // Получаем идентификатор шейдерного объекта (альтернативный вариант)
    GLuint Get()const
    {
        return m_shader;
    }

    // Компилируем ешйдер
    void Compile()
    {
        assert(m_shader != 0);
        glCompileShader(m_shader);
    }

    // Присоединяем шейдерный объект к классу и возвращаем
    // дескриптор ранее присоединенного
    GLuint Attach(GLuint shader)
    {
        GLuint tmp = m_shader;
        m_shader = shader;
        return tmp;
    }

    // Отсоединяем шейдерный объект
    GLuint Detach()
    {
        return Attach(0);
    }

    // Удаляем шейдерный объект
    void Delete()
    {
        assert(m_shader != 0);
        if (m_shader != 0)
        {
            glDeleteShader(Detach());
            m_shader = 0;
        }
    }
};


/*
Шаблонная реализация класса "Шейдерный объект OpenGL"
Параметр t_managed определяет, будет ли время жизни
шейдерного объекта управляться классом CShaderImpl или нет
*/
template <bool t_managed>
class ShaderImpl : public ShaderBase
{
public:
    ShaderImpl(GLuint shader = 0)
        :ShaderBase(shader)
    {
    }

    // Оператор присваивания выполняет
    ShaderImpl& operator=(GLuint shader)
    {
        // Удаляем текущий шейдерный объект только если:
        if (
            t_managed && // Класс управляет шейдерным объектом
            (Get() != 0) && // Задан текущий шейдерный объект
            (Get() != shader) // И он не совпадает с переданным
            )
        {
            Delete();
        }
        Attach(shader);
        return *this;
    }

    // Создаем шейдер заданного типа
    GLuint Create(GLenum type)
    {
        // Удаляем старый шейдерный объект в случае необходимости
        // (если он задан и класс управляет шейдером)
        if ((Get() != 0) && t_managed)
        {
            Delete();
        }
        GLuint shader = glCreateShader(type);
        Attach(shader);
        return shader;
    }

    // Деструктор удалит текущий шейдерный объект (если класс управляет
    // шейдерным объектом)
    ~ShaderImpl(void)
    {
        if (t_managed && (Get() != 0))
        {
            Delete();
        }
    }
};

// Тип "Шейдерный объект"
// (с автоматическим управлением времем жизни
// шейдерного объекта OpenGL)
typedef ShaderImpl<true > Shader;
// Тип "Дескриптор шейдерного объекта"
// (без автоматического управления временем
// жизни шейдерного объекта OpenGL)
typedef ShaderImpl<false> ShaderHandle;


//Класс «Компилятор шейдеров»
class ShaderCompiler
{
public:
    ShaderCompiler(void)
    {
    }

    ~ShaderCompiler(void) {}

    void CompileShader(GLuint shader)
    {
        ShaderHandle sh(shader);
        sh.Compile();
        m_shaders.push_back(shader);
    }

    void CheckStatus()
    {
        std::stringstream strm;
        bool hasErrors = false;

        for (size_t i = 0; i < m_shaders.size(); ++i)
        {
            ShaderHandle shader(m_shaders[i]);
            if (shader.GetParameter(GL_COMPILE_STATUS) != GL_TRUE)
            {
                hasErrors = true;
                strm << "Shader " << shader << " compilation failed: " <<
                    shader.GetInfoLog() << "\n";
            }
        }

        m_shaders.clear();
        if (hasErrors)
        {
            throw std::runtime_error(strm.str());
        }
    }

private:
    ShaderCompiler(ShaderCompiler const&);
    ShaderCompiler& operator=(ShaderCompiler const&);

    std::vector<GLuint> m_shaders;
};


// Класс «Загрузчик шейдеров»
class ShaderLoader
{
public:
    ShaderLoader(void) {}
    ~ShaderLoader(void) {}

    // Выполняем загрузку исходного кода шейдера из файла
    GLuint LoadShader(GLenum shaderType, const wchar_t* fileName, GLuint shaderId = 0)
    {
        std::ifstream stream(fileName);
        if (!stream.is_open())
        {
            throw std::runtime_error("Can't open shader file");
        }
        return LoadShader(shaderType, stream, shaderId);
    }

    // Выполняем загрузку исходного кода шейдера из файла
    GLuint LoadShader(GLenum shaderType, const char* fileName, GLuint shaderId = 0)
    {
        std::ifstream stream(fileName);
        if (!stream.is_open())
        {
            throw std::runtime_error("Can't open shader file");
        }

        return LoadShader(shaderType, stream, shaderId);
    }

    // Выполняем загрузку исходного кода шейдера из входного потока
    GLuint LoadShader(GLenum shaderType, std::istream& stream, GLuint shaderId = 0)
    {
        // Копируем данные из файлового потока в строковый поток
        std::stringstream strStream;
        strStream << stream.rdbuf();

        // Получаем строку, содержащую исходный код шейдера
        // и загружаем шейдер из нее
        return LoadShaderFromString(shaderType, strStream.str(), shaderId);
    }

    // Выполняем загрузку исходного кода шейдера из строки
    GLuint LoadShaderFromString(GLenum shaderType, std::string const& source, GLuint shaderId = 0)
    {
        Shader shader(shaderId);
        // Создаем новый шейдер
        // (если в нам был передан нулевой идентификатор шейдера)
        if (!shader)
        {
            shader.Create(shaderType);
        }
        shader.SetSource(source.c_str());

        // Возвращаем идентификатор шейдерного объекта
        return shader.Detach();
    }
};
