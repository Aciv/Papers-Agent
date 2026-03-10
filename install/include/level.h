#pragma once

#include <cstdint>
#include <type_traits>

namespace Aciv::utility{
	enum class level : std::uint8_t
	{
		none		=	(1 << 0),
		info		=	(1 << 1),
		warning		=	(1 << 2),
		error		=	(1 << 3),
		critical	=	(1 << 4),
		debug		=	(1 << 5),
		all			=	(1 << 6)
	};

	constexpr level operator|(level _lhs, level _rhs)
	{
		return static_cast<level>(
			static_cast<std::underlying_type_t<level>>(_lhs) |
			static_cast<std::underlying_type_t<level>>(_rhs)
			);
	}

	constexpr level operator&(level _lhs, level _rhs)
	{
		return static_cast<level>(
			static_cast<std::underlying_type_t<level>>(_lhs) &
			static_cast<std::underlying_type_t<level>>(_rhs)
			);
	}
	constexpr bool has_level(level _lhs, level _rhs)
	{
		return static_cast<std::underlying_type_t<level>>(_lhs) &
			static_cast<std::underlying_type_t<level>>(_rhs);
	}

	constexpr const char* to_string(level _log_level)
	{
		switch (_log_level)
		{
            case level::info:		return "info";
            case level::warning:	return "warning";
            case level::error:		return "error";
            case level::critical:	return "critical";
            case level::debug:		return "debug";
            case level::all: 		return {};
            case level::none:		return {};
		}
		return {};
	}
}