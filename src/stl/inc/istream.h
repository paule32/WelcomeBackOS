// ----------------------------------------------------------------------------
// \file  istream.h
// \note  (c) 2025, 2026 by Jens Kallup - paule32
//        all rights reserved.
// ----------------------------------------------------------------------------
#ifndef __STL_ISTREAM_H__
#define __STL_ISTREAM_H__

# include "stdint.h"
# include "stl/inc/string.h"

namespace std {
    // -------------------------------
    // Minimal iostream
    // -------------------------------
    struct endl_t {};
    static constexpr endl_t endl{};

    class istream {
    public:
        using read_fn = size_t(*)(char*, size_t);
        read_fn read = nullptr;

        // liest ein "Wort" (whitespace-separated) in std::string
        istream& operator>>(string& out) {
            out.clear();
            if (!read) return *this;

            // skip spaces
            char c = 0;
            while (true) {
                size_t n = read(&c, 1);
                if (n == 0) return *this;     // no data
                if (c != ' ' && c != '\n' && c != '\r' && c != '\t')
                    break;
            }

            // read token
            while (true) {
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                    break;
                out.push_back(c);
                size_t n = read(&c, 1);
                if (n == 0) break;
            }
            return *this;
        }

        // getline minimal
        bool getline(string& out) {
            out.clear();
            if (!read) return false;
            char c = 0;
            while (true) {
                size_t n = read(&c, 1);
                if (n == 0) return (out.size() != 0);
                if (c == '\n') break;
                if (c == '\r') continue;
                out.push_back(c);
            }
            return true;
        }
     };
     
}   // namespace: std

#endif  // __STL_ISTREAM_H__
