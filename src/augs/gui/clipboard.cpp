#include "clipboard.h"
#include "augs/window_framework/platform_utils.h"

namespace augs {
	namespace gui {
		bool clipboard::is_clipboard_own() const {
			ensure(false);
			return false;
		}

		void paste_clipboard_formatted(text::formatted_string& out, text::formatted_char f) {
			ensure(false);
		}

		void clipboard::copy_clipboard(const text::formatted_string& s) {
			ensure(false);
		}

		void clipboard::change_clipboard() {
			ensure(false);
		}
	}
}
