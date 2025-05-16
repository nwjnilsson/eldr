#include <eldr/core/formatter.hpp>

#include <spdlog/pattern_formatter.h>

namespace eldr::logging {

//  class ThreadFormatterFlag : public spdlog::custom_flag_formatter {
//    void format(const spdlog::details::log_msg&,
//                const std::tm&,
//                spdlog::memory_buf_t& dest) override
//    {
//      std::string some_txt{ eldr::Thread::thread()->name() };
//      dest.append(some_txt.data(), some_txt.data() + some_txt.size());
//    }
//    std::unique_ptr<custom_flag_formatter> clone() const override
//    {
//      return spdlog::details::make_unique<ThreadFormatterFlag>();
//    }
//  };
//
DefaultFormatter::DefaultFormatter() : Formatter()
{
  auto formatter{ std::make_unique<spdlog::pattern_formatter>() };
  //  [time] [Logger name] [color+level+endcolor] message
  formatter->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] %v");
  formatter_ = std::move(formatter);
}
} // namespace eldr::logging
