#include "LogManager.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include "libInfinite/logger/logger.h"

#include <stdint.h>
#include <gtkmm.h>

LogManager::LogManager(uint32_t size, int consoleOutputLevel, Gtk::TextBuffer* textBuffer){
	// first allocate the buffer
	buffer = (char*)malloc(size);
	// for now, the first message starts at the start of the buffer
	start = 0;
	max = size-1;	// the last valid offset in the buffer.

	// the rptr and wptr also start at the beginning
	rptr = 0;
	wptr = 0;
	consoleOutput = consoleOutputLevel;
	this->textBuffer = textBuffer;
	view = nullptr;

	// we might be able to set up the tags
	if(textBuffer != nullptr){
		createTags();
	}
}

void LogManager::createTags(){
	// debug is just gray
	debugTag = textBuffer->create_tag("debug");
	debugTag->set_property("foreground-rgba", Gdk::RGBA("Gray"));

	// info just stays like normal
	infoTag = textBuffer->create_tag("info");

	// warning is orange red
	warningTag = textBuffer->create_tag("warning");
	warningTag->set_property("foreground-rgba", Gdk::RGBA("Orange Red"));

	// error is red
	errorTag = textBuffer->create_tag("error");
	errorTag->set_property("foreground-rgba", Gdk::RGBA("Red"));

	// critical is bold crimson
	criticalTag = textBuffer->create_tag("critical");
	criticalTag->set_property("foreground-rgba", Gdk::RGBA("Crimson"));
	criticalTag->set_property("weight", Pango::WEIGHT_BOLD);
}

LogManager::~LogManager(){

}

void LogManager::setTextBuffer(Gtk::TextBuffer* textBuffer){
	this->textBuffer = textBuffer;
	createTags();
}

void LogManager::setTextView(Gtk::TextView* view){
	this->view = view;
}

void LogManager::submitLog(const char* buf, uint32_t len){
	// check if we run past the end of the buffer. If we do, we have to copy the string in two steps

	if(wptr + len > max){
		// we go past the end of the buffer
		uint32_t part1 = max - wptr;	// length of the first part, the data that still fits in the end of the buffer
		memcpy(buffer + wptr, buf, part1);	// copy the first part
		memcpy(buffer,buf+part1,len-part1);	// copy the rest of the message
		wptr = len-part1;	// we overrun the end of the buffer and looped back to the start, so we don't really need to consider the last wptr anymore here
		start = wptr + 1;	// we moved the start of the log in the buffer, since we overwrote old data
	} else {
		// we don't go past the end of the buffer
		memcpy(buffer + wptr, buf, len);
		wptr += len;	// update the write pointer
	}
	int level = (uint8_t) *buf;	// get the log level to check if we should output it to the console
	if(level >= consoleOutput){
		// this message should get written to console
		// set the color
		switch(level){
		case LOG_LEVEL_DEBUG:
			printf("\x1b[0m\x1b[2;37m");	// faint, gray
			break;
		case LOG_LEVEL_INFO:
			printf("\x1b[0m");	// just regular text
			break;
		case LOG_LEVEL_WARNING:
			printf("\x1b[0m\x1b[33m");	// yellow
			break;
		case LOG_LEVEL_ERROR:
			printf("\x1b[0m\x1b[31m");	// red
			break;
		case LOG_LEVEL_CRITICAL:
			printf("\x1b[0m\x1b[31;1m");	// red, bold
			break;
		}
		printf("%s\x1b[0m",buf+1);	// print out the message and reset the formatting
	}

	// just immediately update the text buffer for now
	updateBuffer();
}

Glib::RefPtr<Gtk::TextTag> LogManager::getTag(int level){
	switch(level){
	case LOG_LEVEL_DEBUG:
		return debugTag;
	case LOG_LEVEL_INFO:
		return infoTag;
	case LOG_LEVEL_WARNING:
		return warningTag;
	case LOG_LEVEL_ERROR:
		return errorTag;
	case LOG_LEVEL_CRITICAL:
		return criticalTag;
	default:
		// info in the default case, as it's unstyled. Critical might be better though, as this shouldn't happen
		return infoTag;
	}
}

void LogManager::updateBuffer(){
	if(wptr == rptr){
		// nothing to do, wptr and rptr are the same, so nothing got changed
		// this could also happen if exactly a multiple of the ring buffers size got written, but that's unlikely
		return;
	}

	if(textBuffer == nullptr){
		// the textBuffer hasn't been set yet, so we won't do anything yet. The rptr will be updated once the textBuffer has been set
		return;
	}

	// there is a message that hasn't been displayed yet, and the buffer may or may not have looped
	// so we first check if the buffer looped
	if(wptr < rptr){
		// now the buffer looped over, and we have to handle that with more checks
		while(rptr > wptr){
			// up until we've looped over the end of the buffer
			uint32_t len = strnlen((const char*)(buffer + rptr),max - rptr);
			uint8_t level = (uint8_t)*(buffer + rptr);	// get the level of this entry
			if(len == max - rptr && *buffer != 0){
				// this message gets split up, and not just the trailing null byte is missing, so we have to do this in two parts
				textBuffer->insert_with_tag(textBuffer->end(),(const char*)(buffer + rptr + 1),(const char*)(buffer + max),getTag(level));	// insert the first part
				len = strnlen((const char*)(buffer),wptr);	// length of the second part
				textBuffer->insert_with_tag(textBuffer->end(),(const char*)(buffer),(const char*)(buffer + len),getTag(level));	// insert the second part
				rptr = len + 1;	// update the rptr
			} else{
				// this message doesn't wrap yet, so handle it like normal
				textBuffer->insert_with_tag(textBuffer->end(),(const char*)(buffer + rptr + 1),(const char*)(buffer + rptr + len),getTag(level));	// insert the text
				rptr = len + 1;	// update the rptr
				rptr %= max;	// in case the message didn't wrap, but just perfectly fit into the buffer, or only the null byte got wrapped
			}
		}
	}

	// either the buffer didn't loop, or we now displayed all messages that were at the end of the buffer
	if(wptr > rptr){
		// the buffer didn't loop over, so this should be pretty simple, just copy in the new data
		// since we have to handle the log level separately, this has to be done individually for each entry
		while(rptr < wptr){
			uint32_t len = strnlen((const char*)(buffer + rptr),wptr - rptr);
			uint8_t level = (uint8_t)*(buffer + rptr);	// get the level of this entry
			textBuffer->insert_with_tag(textBuffer->end(),(const char*)(buffer + rptr + 1),(const char*)(buffer + rptr + len),getTag(level));	// insert the text
			//printf("Message: %.*s",len-1,buffer+1);
			rptr += len + 1;	// update the read pointer
		}
	}

	if(view != nullptr){
		// we have a TextView we can scroll down in
		Glib::RefPtr<Gtk::TextMark> mark = textBuffer->create_mark(textBuffer->end(), false);	// Oh hi mark
		view->scroll_to(mark, 0);
		textBuffer->delete_mark(mark);	// Oh bye mark
	}
}
