#pragma once

#include "ww/Window.h"
#include "ww/VBox.h"
#include "ww/HBox.h"
#include "ww/Button.h"
#include "ww/HLine.h"
#include "ww/Win32/MessageLoop.h"

#include <vector>
#include <map>

namespace ww{

	class Button;

	enum Response{
		RESPONSE_NO, //  = 7
		RESPONSE_YES, //  = 6
		RESPONSE_CANCEL, //  = 2
		RESPONSE_OK, //  = 1
		RESPONSE_SAVE, //  
		RESPONSE_ABORT, //  = 3
		RESPONSE_IGNORE, //  = 5
		RESPONSE_RETRY // = 4
	};

	class WW_API Dialog : public ww::Window
	{
	public:
		Dialog(ww::Widget* owner, int border = 12);
		Dialog(HWND owner, int border = 12);
		~Dialog();

		int showModal();
		virtual void onResponse(int response);

		void add(Widget* widget, PackMode packMode);
		void addButton(const char* label, int response, bool atRight = true);
		void setDefaultResponse(int response);
		void setCancelResponse(int response);

		void onClose();
		int cancelResponse() const { return cancelResponse_; }
		int defaultResponse() const { return defaultResponse_; }

		void serialize(Archive& ar);

		HWND parentWnd() const{ return parentWnd_; }
	protected:
		void _onWMCommand(int command);
		void interruptModalLoop();
		virtual void onKeyDefault();
		virtual void onKeyCancel();

		void init(int border);

		VBox* vbox_;
		HBox* hboxButtons_;

		
		int response_;
		int cancelResponse_;
		int defaultResponse_;

		typedef std::map<Response, Button*> Map;
		Map map_;
	private:
		Win32::MessageLoop* activeDialogLoop_;
		HWND parentWnd_;
	};

}

