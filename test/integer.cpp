// Copyright 2017-2019 Elias Kosunen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This file is a part of scnlib:
//     https://github.com/eliaskosunen/scnlib

#include <doctest.h>
#include <scn/scn.h>

namespace detail {
    template <typename CharT>
    std::basic_string<CharT> widen(std::string)
    {
    }

    template <>
    std::basic_string<char> widen(std::string str)
    {
        return str;
    }

    template <>
    std::basic_string<wchar_t> widen(std::string str)
    {
        return std::wstring(str.begin(), str.end());
    }
}  // namespace detail

template <typename CharT, typename T>
static scn::error scan_value(scn::method m,
                             std::string source,
                             std::string f,
                             T& value)
{
    auto stream = scn::make_stream(detail::widen<CharT>(source));
    auto fstr = detail::widen<CharT>(f);
    return scn::scan(scn::options::builder{}.int_method(m), stream,
                     scn::basic_string_view<CharT>(fstr.data(), fstr.size()),
                     value);
}

template <typename CharT, typename T>
struct intpair {
    using char_type = CharT;
    using value_type = T;
};

TEST_CASE_TEMPLATE_DEFINE("integer", T, integer_test)
{
    using value_type = typename T::value_type;
    using char_type = typename T::char_type;

    const bool u = std::is_unsigned<value_type>::value;

    std::array<scn::method, 3> methods{scn::method::sto, scn::method::strto,
                                       scn::int_from_chars_if_available()};

    for (auto&& method : methods) {
        {
            auto m = static_cast<int>(method);
            CAPTURE(m);
        }

        {
            value_type i{1};
            auto e = scan_value<char_type>(method, "0", "{}", i);
            CHECK(i == 0);
            CHECK(e);
        }
        {
            value_type i{};
            auto e = scan_value<char_type>(method, "1", "{}", i);
            CHECK(i == 1);
            CHECK(e);
        }

        if (!u) {
            value_type i{};
            auto e = scan_value<char_type>(method, "-1", "{}", i);
            CHECK(i == -1);
            CHECK(e);
        }
        else {
            value_type i{};
            auto e = scan_value<char_type>(method, "-1", "{}", i);
            CHECK(e.get_code() == scn::error::value_out_of_range);
        }

        const bool can_fit_2pow31 = []() {
            if (u) {
                return sizeof(value_type) >= 4;
            }
            return sizeof(value_type) >= 8;
        }();
        if (can_fit_2pow31) {
            value_type i{};
            auto e = scan_value<char_type>(method, "2147483648", "{}", i);
            CHECK(i == 2147483648);
            CHECK(e);
        }
        else {
            value_type i{};
            auto e = scan_value<char_type>(method, "2147483648", "{}", i);
            CHECK(e.get_code() == scn::error::value_out_of_range);
        }

        {
            value_type i{};
            auto e = scan_value<char_type>(method, "1011", "{b2}", i);
            CHECK(i == 11);
            CHECK(e);
        }
        {
            value_type i{};
            auto e = scan_value<char_type>(method, "400", "{o}", i);
            CHECK(i == 0400);
            CHECK(e);
        }
        {
            value_type i{};
            auto e = scan_value<char_type>(method, "0400", "{}", i);
            CHECK(i == 0400);
            CHECK(e);
        }

        const bool can_fit_badidea = []() { return sizeof(value_type) >= 4; }();
        if (can_fit_badidea) {
            value_type i{};
            auto e = scan_value<char_type>(method, "bad1dea", "{x}", i);
            CHECK(i == 0xbad1dea);
            CHECK(e);
        }
        else {
            value_type i{};
            auto e = scan_value<char_type>(method, "bad1dea", "{x}", i);
            CHECK(e.get_code() == scn::error::value_out_of_range);
        }
        if (can_fit_badidea) {
            value_type i{};
            auto e = scan_value<char_type>(method, "0xbad1dea", "{}", i);
            CHECK(i == 0xbad1dea);
            CHECK(e);
        }
        else {
            value_type i{};
            auto e = scan_value<char_type>(method, "0xbad1dea", "{}", i);
            CHECK(e.get_code() == scn::error::value_out_of_range);
        }
    }
}

template <typename T>
using char_intpair = intpair<char, T>;
template <typename T>
using wchar_intpair = intpair<wchar_t, T>;

TYPE_TO_STRING(char_intpair<short>);
TYPE_TO_STRING(char_intpair<int>);
TYPE_TO_STRING(char_intpair<long>);
TYPE_TO_STRING(char_intpair<long long>);
TYPE_TO_STRING(char_intpair<unsigned short>);
TYPE_TO_STRING(char_intpair<unsigned int>);
TYPE_TO_STRING(char_intpair<unsigned long>);
TYPE_TO_STRING(char_intpair<unsigned long long>);
TYPE_TO_STRING(wchar_intpair<int>);
TYPE_TO_STRING(wchar_intpair<long>);
TYPE_TO_STRING(wchar_intpair<long long>);
TYPE_TO_STRING(wchar_intpair<unsigned int>);
TYPE_TO_STRING(wchar_intpair<unsigned long>);
TYPE_TO_STRING(wchar_intpair<unsigned long long>);

TEST_CASE_TEMPLATE_INSTANTIATE(integer_test,
                               char_intpair<short>,
                               char_intpair<int>,
                               char_intpair<long>,
                               char_intpair<long long>,
                               char_intpair<unsigned short>,
                               char_intpair<unsigned int>,
                               char_intpair<unsigned long>,
                               char_intpair<unsigned long long>,
                               wchar_intpair<int>,
                               wchar_intpair<long>,
                               wchar_intpair<long long>,
                               wchar_intpair<unsigned int>,
                               wchar_intpair<unsigned long>,
                               wchar_intpair<unsigned long long>);
