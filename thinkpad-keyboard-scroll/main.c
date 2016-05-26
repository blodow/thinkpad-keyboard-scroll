//
//  thinkpad-keyboard-scroll
//
//  Copyright © 2016 Nico Blodow. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright notice,
//  this list of conditions and the following disclaimer.
//  
//  2. Redistributions in binary form must reproduce the above copyright
//  notice, this list of conditions and the following disclaimer in the
//  documentation and/or other materials provided with the distribution.
//  
//  3. Neither the name of the copyright holder nor the names of its
//  contributors may be used to endorse or promote products derived from this
//  software without specific prior written permission.
// 
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//

#include <ApplicationServices/ApplicationServices.h>

// Application state
static bool maybeScrolling = false;
static bool scrolling = false;
static CGPoint scrollLocation;

CGEventRef eventCallback(CGEventTapProxy proxy, CGEventType type,
                         CGEventRef event, void *hint)
{
    // TODO: make sure we only treat these events for the Thinkpad USB keyboard's
    // middle button
    if (type == kCGEventOtherMouseDown) {
        maybeScrolling = true;
    } else if ((type == kCGEventOtherMouseDragged) && maybeScrolling) {
        if (!scrolling) {
            // first scroll event
            scrolling = true;
            scrollLocation = CGEventGetLocation(event);
        }
        // determine scroll delta
        CGEventSourceRef s = CGEventCreateSourceFromEvent(event);
        uint64_t deltaX = CGEventGetIntegerValueField(event, kCGMouseEventDeltaX);
        uint64_t deltaY = CGEventGetIntegerValueField(event, kCGMouseEventDeltaY);
        // force mouse cursor to stay fixed
        CGWarpMouseCursorPosition(scrollLocation);
        // overwrite mouse movement event with scroll wheel event
        event = CGEventCreateScrollWheelEvent(s,
                                              kCGScrollEventUnitPixel,
                                              2,
                                              (int)-deltaY*10,
                                              (int)-deltaX*10);
    } else if (type == kCGEventOtherMouseUp) {
        // if maybeScrolling is true, but scrolling is false, no scrolling was
        // performed, so we let it pass as a click. if scrolling is true, we
        // overwrite the event with NULL.
        if (scrolling) {
            event = NULL;
        }
        maybeScrolling = false;
        scrolling = false;
    }
    
    return event;
}

int main(void)
{
    CGEventMask mask = (1 << kCGEventOtherMouseUp)
    | (1 << kCGEventOtherMouseDown)
    | (1 << kCGEventOtherMouseDragged);
    
    CFMachPortRef tap = CGEventTapCreate(kCGHIDEventTap,
                                         kCGHeadInsertEventTap,
                                         0,
                                         mask,
                                         eventCallback,
                                         NULL);
    
    if (!tap) {
        fprintf(stderr, "could not create event tap\n");
        exit(EXIT_FAILURE);
    }
    
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, tap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(tap, true);
    CFRunLoopRun();
    
    // In a real program, one would have arranged for cleaning up.
    
    exit(EXIT_SUCCESS);
}
