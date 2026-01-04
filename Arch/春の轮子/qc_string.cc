/**
 * @file qc_string.cc
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2024-08-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <cctype>
#include <cstddef>
#include <memory>
#include <cstring>
// 借鉴MSVC中string的实现
// 使用空间换时间,开16字节的缓冲区
namespace qc {
class string {
public:
    string() noexcept {
        m_size = 1;
        m_buffer[0] = '\0';
        m_data = m_buffer;
    }
    string(char const* str) {
        m_size = strlen(str);
        if (m_size >= 16) {
            m_data = new char[m_size + 1];
            strcpy(m_data, str);
        } else {
            strcpy(m_buffer, str);
            m_data = m_buffer;
        }
    }

    string(string const& str) {
        m_size = str.m_size;
        if (m_size >= 16) {
            m_data = new char[m_size + 1];
            strcpy(m_data, str.m_data);
        } else {
            strcpy(m_buffer, str.m_data);
            m_data = m_buffer;
        }
    }

    string(string&& that) noexcept {
        m_size = that.m_size;
        if (m_size >= 16) {
            m_data = that.m_data;
            that.m_data = nullptr;
            that.m_size = 0;
        } else {
            strcpy(m_buffer, that.m_buffer);
            m_data = m_buffer;
            that.m_data = nullptr;
            that.m_size = 0;
        }
    }

    string& operator=(string const& str) {
        if (m_data == str.m_data) return *this;
        if (m_size >= 16) delete[] m_data;
        m_data = nullptr;
        m_size = str.m_size;
        if (m_size >= 16) {
            m_data = new char[m_size + 1];
            strcpy(m_data, str.m_data);
        } else {
            strcpy(m_buffer, str.m_data);
            m_data = m_buffer;
        }
    }

    string& operator=(string && str) {
        if (m_data == str.m_data) return *this;
        if (m_size >= 16) delete[] m_data;
        m_data = nullptr;
        m_size = str.m_size;
        if (m_size >= 16) {
            m_data = str.m_data;
            str.m_size = 0;
            str.m_data = nullptr;
        } else {
            strcpy(m_buffer, str.m_data);
            m_data = m_buffer;
            str.m_data = nullptr;
            str.m_size = 0;
        }
    }

    ~string() {
        if (m_size >= 16) delete[] m_data;
    }
public:
    string& operator+=(string const& str) {
        m_size += str.m_size;
        if (m_size >= 16) {
            char *data = new char[m_size];
            strcpy(data, m_data);
            strcat(data, str.m_data);
            delete[] m_data;
            m_data = data;
            return *this;
        } else {
            strcat(m_buffer, str.m_buffer);
            m_data = m_buffer;
        }
    }

    string& operator+=(char const* s) {
        m_size += strlen(s);
        if (m_size >= 16) {
            char *data = new char[m_size];
            strcpy(data, m_data);
            strcat(data, s);
            delete[] m_data;
            m_data = data;
            return *this;
        } else {
            strcat(m_buffer, s);
            m_data = m_buffer;
        }
    }

    string& operator+=(char c) {
        m_size += 1;
        if (m_size >= 16) {
            char *data = new char[m_size];
            strcpy(data, m_data);
            strcat(data, &c);
            delete[] m_data;
            m_data = data;
            return *this;
        } else {
            strcat(m_buffer, &c);
            m_data = m_buffer;
        }
    }

private:
    char m_buffer[16];
    char* m_data;
    size_t m_size;
};

}