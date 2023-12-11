// Copyright 2017 Elias Kosunen
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

#pragma once

#include <scn/detail/args.h>
#include <scn/detail/locale_ref.h>
#include <scn/detail/scan_buffer.h>
#include <scn/util/string_view.h>

namespace scn {
    SCN_BEGIN_NAMESPACE

    namespace detail {
        template <typename Iterator, typename = void>
        struct is_comparable_with_nullptr : std::false_type {};
        template <typename Iterator>
        struct is_comparable_with_nullptr<
            Iterator,
            std::void_t<decltype(SCN_DECLVAL(const Iterator&) == nullptr)>>
            : std::true_type {};
    }  // namespace detail

    /**
     * \defgroup ctx Contexts and scanners
     *
     * \brief Lower-level APIs used for scanning individual values
     */

    /**
     * Scanning context.
     *
     * \ingroup ctx
     */
    template <typename CharT>
    class basic_scan_context {
    public:
        /// Character type of the input
        using char_type = CharT;
        using buffer_type = detail::basic_scan_buffer<char_type>;
        using range_type = typename buffer_type::range_type;
        using iterator = ranges::iterator_t<range_type>;
        using sentinel = ranges::sentinel_t<range_type>;
        using parse_context_type = basic_scan_parse_context<char_type>;

        using contiguous_range_type =
            typename buffer_type::contiguous_range_type;
        using contiguous_iterator = ranges::iterator_t<contiguous_range_type>;

        using arg_type = basic_scan_arg<basic_scan_context>;

        /**
         * The scanner type associated with this scanning context.
         */
        template <typename T>
        using scanner_type = scanner<T, char_type>;

        constexpr basic_scan_context(buffer_type& buf,
                                     basic_scan_args<basic_scan_context> a,
                                     detail::locale_ref loc = {})
            : m_buffer(buf),
              m_current(buf.get_forward_buffer().begin()),
              m_args(SCN_MOVE(a)),
              m_locale(loc)
        {
        }

        constexpr basic_scan_context(iterator curr,
                                     basic_scan_args<basic_scan_context> a,
                                     detail::locale_ref loc = {})
            : m_buffer(*curr.parent()),
              m_current(curr),
              m_args(SCN_MOVE(a)),
              m_locale(loc)
        {
        }

        basic_scan_context(const basic_scan_context&) = delete;
        basic_scan_context& operator=(const basic_scan_context&) = delete;

        basic_scan_context(basic_scan_context&&) = default;
        basic_scan_context& operator=(basic_scan_context&&) = default;
        ~basic_scan_context() = default;

        /// Get argument at index `id`
        constexpr basic_scan_arg<basic_scan_context> arg(size_t id) const
            SCN_NOEXCEPT
        {
            return m_args.get(id);
        }

        constexpr const basic_scan_args<basic_scan_context>& args() const
        {
            return m_args;
        }

        constexpr iterator begin() const
        {
            return m_current;
        }

        constexpr sentinel end() const
        {
            return ranges_std::default_sentinel;
        }

        constexpr auto range() const
        {
            return ranges::subrange{begin(), end()};
        }

        /// Advances the beginning of the input range to `it`
        void advance_to(iterator it)
        {
            if constexpr (detail::is_comparable_with_nullptr<iterator>::value) {
                if (it == nullptr) {
                    it = end();
                }
            }
            m_current = SCN_MOVE(it);
        }

        void advance_to(contiguous_iterator it)
        {
            SCN_EXPECT(m_buffer.is_contiguous());
            auto n = ranges::distance(
                m_current.to_contiguous_segment_iterator(), it);
            SCN_EXPECT(n >= 0);
            m_current.unsafe_advance(n);
        }

        SCN_NODISCARD constexpr detail::locale_ref locale() const
        {
            return m_locale;
        }

        buffer_type& internal_buffer()
        {
            return m_buffer;
        }
        const buffer_type& internal_buffer() const
        {
            return m_buffer;
        }

    private:
        buffer_type& m_buffer;
        iterator m_current;
        basic_scan_args<basic_scan_context> m_args;
        detail::locale_ref m_locale;
    };

    SCN_END_NAMESPACE
}  // namespace scn
