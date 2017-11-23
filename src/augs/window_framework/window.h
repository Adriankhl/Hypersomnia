#pragma once
#if PLATFORM_WINDOWS
#include <Windows.h>
#undef min
#undef max
#elif PLATFORM_UNIX
/*
	Hacky forward declarations to avoid the need for X includes.
	There are errors anyway when the declarations are conflicting,
	so we'll know when something's wrong.
*/
#include <cstdint>

struct __GLXcontextRec;
typedef struct __GLXcontextRec *GLXContext;

typedef unsigned long XID;
typedef XID GLXWindow;
typedef XID GLXDrawable;

typedef uint32_t xcb_window_t;

struct _XDisplay;
typedef struct _XDisplay Display;
struct xcb_connection_t;
typedef uint32_t xcb_timestamp_t;

struct _XCBKeySymbols;
typedef struct _XCBKeySymbols xcb_key_symbols_t;
#endif

#include <optional>

#include "augs/math/rects.h"
#include "augs/math/vec2.h"

#include "augs/templates/settable_as_current_mixin.h"
#include "augs/templates/exception_templates.h"

#include "augs/misc/timing/timer.h"
#include "augs/misc/machine_entropy.h"

#include "augs/window_framework/event.h"
#include "augs/window_framework/window_settings.h"

namespace augs {
	struct window_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	// extern LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);
	class window : public settable_as_current_mixin<window> {
#if PLATFORM_WINDOWS
		friend LRESULT CALLBACK wndproc(HWND, UINT, WPARAM, LPARAM);

		HWND hwnd = nullptr;
		HDC hdc = nullptr;
		HGLRC hglrc = nullptr;

		vec2i min_window_size;
		vec2i max_window_size;

		int style = 0xdeadbeef;
		int exstyle = 0xdeadbeef;

		bool double_click_occured = false;
		bool clear_window_inputs_once = true;

		timer triple_click_timer;
		unsigned triple_click_delay = 0xdeadbeef; /* maximum delay time for the next click (after doubleclick) to be considered tripleclick (in milliseconds) */

		void show();

		std::optional<event::change> handle_event(
			const UINT, 
			const WPARAM, 
			const LPARAM
		);

#elif PLATFORM_UNIX
		xcb_timestamp_t last_ldown_time_ms = 0;

		GLXContext context = 0;
		GLXWindow glxwindow = 0;
		xcb_window_t window_id = 0;
		GLXDrawable drawable = 0;
		Display *display = nullptr;
		xcb_connection_t *connection = nullptr;

		xcb_key_symbols_t* syms = nullptr;
		
		int raw_mouse_input_fd = -1;
#else
#error "Unsupported platform!"
#endif

		bool active = false;

		vec2i last_mouse_pos;
		bool mouse_pos_paused = false;

		std::optional<event::change> handle_mousemove(
			const basic_vec2<short> new_position
		);
		
		event::change do_raw_motion(const basic_vec2<short>);
		std::optional<event::change> sync_mouse_on_click_activate(const event::change&);
		void common_event_handler(event::change, local_entropy&);

		using settable_as_current_base = settable_as_current_mixin<window>;
		friend settable_as_current_base;

		bool set_as_current_impl();
		static void set_current_to_none_impl();

		void set_window_name(const std::string& name);
		void set_window_border_enabled(const bool);
		void set_window_rect(const xywhi);

		void set_fullscreen_geometry(const bool);
		void set_fullscreen_hint(const bool);

		window_settings current_settings;

		void destroy();
	public:
		window(const window_settings&);
		~window();

		window(window&&) = delete;
		window& operator=(window&&) = delete;

		window(const window&) = delete;
		window& operator=(const window&) = delete;

		bool swap_buffers();

		void set_mouse_pos_paused(const bool);
		bool is_mouse_pos_paused() const;

		void apply(const window_settings&, const bool force = false);
		void sync_back_into(window_settings&);
		window_settings get_current_settings() const;

		local_entropy collect_entropy();
		void collect_entropy(local_entropy& into);

		vec2i get_screen_size() const;
		xywhi get_window_rect() const;

		void set_cursor_pos(vec2i);
		void clip_system_cursor();
		void disable_cursor_clipping();
		void set_cursor_visible(bool flag); 

		bool is_active() const;

		struct file_dialog_filter {
			std::string description;
			std::string extension;
		};

		std::optional<std::string> open_file_dialog(
			const std::vector<file_dialog_filter>& filters,
			std::string custom_title = std::string()
		) const;

		std::optional<std::string> save_file_dialog(
			const std::vector<file_dialog_filter>& filters,
			std::string custom_title = std::string()
		) const;
	};
}
