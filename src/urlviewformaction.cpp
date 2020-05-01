#include "urlviewformaction.h"

#include <sstream>

#include "config.h"
#include "fmtstrformatter.h"
#include "listformatter.h"
#include "rssfeed.h"
#include "strprintf.h"
#include "utils.h"
#include "view.h"

namespace newsboat {

/*
 * The UrlViewFormAction is probably the simplest dialog of all. It
 * displays a list of URLs, and makes it possible to open the URLs
 * in a browser or to bookmark them.
 */

UrlViewFormAction::UrlViewFormAction(View* vv,
	std::shared_ptr<RssFeed>& feed,
	std::string formstr,
	ConfigContainer* cfg)
	: FormAction(vv, formstr, cfg)
	, quit(false)
	, feed(feed)
	, urls_list("urls", FormAction::f)
{
}

UrlViewFormAction::~UrlViewFormAction() {}

bool UrlViewFormAction::process_operation(Operation op,
	bool /* automatic */,
	std::vector<std::string>* /* args */)
{
	bool hardquit = false;
	switch (op) {
	case OP_SK_UP:
		urls_list.move_up();
		break;
	case OP_SK_DOWN:
		urls_list.move_down();
		break;
	case OP_SK_HOME:
		urls_list.move_to_first();
		break;
	case OP_SK_END:
		urls_list.move_to_last();
		break;
	case OP_SK_PGUP:
		urls_list.move_page_up();
		break;
	case OP_SK_PGDOWN:
		urls_list.move_page_down();
		break;
	case OP_OPENINBROWSER:
	case OP_OPEN: {
		std::string posstr = f->get("urls_pos");
		if (posstr.length() > 0) {
			unsigned int idx = utils::to_u(posstr, 0);
			v->set_status(_("Starting browser..."));
			v->open_in_browser(links[idx].first);
			v->set_status("");
		} else {
			v->show_error(_("No link selected!"));
		}
	}
	break;
	case OP_BOOKMARK: {
		std::string posstr = f->get("urls_pos");
		if (posstr.length() > 0) {
			unsigned int idx = utils::to_u(posstr, 0);

			this->start_bookmark_qna(
				"", links[idx].first, "", feed->title());

		} else {
			v->show_error(_("No link selected!"));
		}
	}
	break;
	case OP_1:
	case OP_2:
	case OP_3:
	case OP_4:
	case OP_5:
	case OP_6:
	case OP_7:
	case OP_8:
	case OP_9:
	case OP_0: {
		unsigned int idx = op - OP_1;

		if (idx < links.size()) {
			v->set_status(_("Starting browser..."));
			v->open_in_browser(links[idx].first);
			v->set_status("");
		}
	}
	break;
	case OP_QUIT:
		quit = true;
		break;
	case OP_HARDQUIT:
		hardquit = true;
		break;
	default: // nothing
		break;
	}
	if (hardquit) {
		while (v->formaction_stack_size() > 0) {
			v->pop_current_formaction();
		}
	} else if (quit) {
		v->pop_current_formaction();
	}
	return true;
}

void UrlViewFormAction::prepare()
{
	if (do_redraw) {
		ListFormatter listfmt;
		unsigned int i = 0;
		for (const auto& link : links) {
			listfmt.add_line(
				strprintf::fmt("%2u  %s", i + 1, link.first),
				i);
			i++;
		}
		f->modify("urls", "replace_inner", listfmt.format_list());
		urls_list.set_lines(listfmt.get_lines_count());
	}
}

void UrlViewFormAction::init()
{
	v->set_status("");

	f->run(-3); // compute all widget dimensions

	std::string viewwidth = f->get("urls:w");
	unsigned int width = utils::to_u(viewwidth, 80);

	FmtStrFormatter fmt;
	fmt.register_fmt('N', PROGRAM_NAME);
	fmt.register_fmt('V', utils::program_version());

	f->set("head",
		fmt.do_format(
			cfg->get_configvalue("urlview-title-format"), width));
	do_redraw = true;
	quit = false;
	set_keymap_hints();
}

KeyMapHintEntry* UrlViewFormAction::get_keymap_hint()
{
	static KeyMapHintEntry hints[] = {{OP_QUIT, _("Quit")},
		{OP_OPEN, _("Open in Browser")},
		{OP_BOOKMARK, _("Save Bookmark")},
		{OP_NIL, nullptr}
	};
	return hints;
}

void UrlViewFormAction::handle_cmdline(const std::string& cmd)
{
	unsigned int idx = 0;
	if (1 == sscanf(cmd.c_str(), "%u", &idx)) {
		if (idx < 1 || idx > links.size()) {
			v->show_error(_("Invalid position!"));
		} else {
			f->set("urls_pos", std::to_string(idx - 1));
		}
	} else {
		FormAction::handle_cmdline(cmd);
	}
}

std::string UrlViewFormAction::title()
{
	return _("URLs");
}

} // namespace newsboat
