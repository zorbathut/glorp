#ifndef GLORP_INPUT
#define GLORP_INPUT

// Includes
#include <string>
#include <vector>
using namespace std;


namespace Glorp {
  // GlopKey devices
  const int kDeviceKeyboard = -1;

  // Key repeat-rate constants
  const int kRepeatDelay = 500;  // Time (in ms) between key down event #1 and #2 while a key is down
  const int kRepeatRate = 60;    // Time (in ms) between later key down events while a key is down

  // This is a basic identifier for any kind of key.
  // To create a Key, use the static methods and constants below.
  struct Key {
    // Constructor
    Key(int _index = -2, int _device = kDeviceKeyboard): index(_index), device(_device) {}
    short index, device;

    // Comparators
    bool operator==(const Key &rhs) const {
      return device == rhs.device && index == rhs.index;
    }
    bool operator!=(const Key &rhs) const {
      return device != rhs.device || index != rhs.index;
    }
    bool operator<(const Key &rhs) const {
      return (device == rhs.device? device < rhs.device : index < rhs.index);
    }

    // Key property queries. Most are self-explanatory, but:
    //  GetName: Returns a string description of the key. For example, "Enter" or
    //           "Joystick #1 - Button #4".
    //  IsTrackable: All "keys" generate key press events when they are first pressed. Our other
    //               functionality (IsKeyDown, GetKeyPressAmount, and repeat events) requires being
    //               able to track the key's state. This returns whether that is possible on all
    //               OSes.
    //  IsDerivedKey: Certain Keys do not correspond to a single key - for example,
    //                kKeyEitherShift or any key with kAnyJoystick. This returns whether that applies
    //                to this key.
    //  IsMouseMotion: Includes mouse wheel.
    //  IsMotionKey: Does this key correspond to something that a user likely would consider more
    //               like scrolling or moving a pointer, rather than an actual key? Currently, this
    //               is mouse motion and joystick axes.
    //  IsModifierKey: Is this a non-motion key that a user would probably still not consider to
    //                 be a "real" key. Currently: alt, shift, and control.
    //  GetJoystickAxisNum, etc.: Assuming this is a joystick axis key of some kind, which axis is
    //                            it tied to?
    const string GetName() const;
    bool IsTrackable() const;
    bool IsDerivedKey() const;
    bool IsKeyboardKey() const;
    bool IsMouseKey() const;
    bool IsJoystickKey() const;
    bool IsMouseMotion() const;
    bool IsJoystickAxis() const;
    bool IsJoystickAxisPos() const;
    bool IsJoystickAxisNeg() const;
    bool IsJoystickHat() const;
    bool IsJoystickHatUp() const;
    bool IsJoystickHatRight() const;
    bool IsJoystickHatDown() const;
    bool IsJoystickHatLeft() const;
    bool IsJoystickButton() const;
    bool IsMotionKey() const;
    bool IsModifierKey() const;
    int GetJoystickAxisNumber() const;
    int GetJoystickHatNumber() const;
    int GetJoystickButtonNumber() const;
  };

  namespace Keys {
    // Key constants - miscellaneous
    const Key NoKey(-2);
    const Key AnyKey(-1);

    // Key constants - keyboard. General rules:
    //  - If a key has a clear ASCII value associated with it, its Key is given just by that ASCII
    //    value. For example, Enter is 13.
    //  - The non-shift, non-caps lock ASCII value is used when there's ambiguity. Thus, 'a' and ','
    //    are valid Keys, but 'A' and '<' are not valid Keys.
    //  - Key-pad keys are NEVER given Keys based on their ASCII values.
    // Note: these ASCII-related key indices are not intended to replace ASCII. If you want an ASCII
    // value, call Input::ToAscii so that shift, num lock and caps lock are all considered.
    const Key Backspace(8);
    const Key Tab(9);
    const Key Enter(13);
    const Key Return(13);
    const Key Escape(27);

    const Key F1(129);
    const Key F2(130);
    const Key F3(131);
    const Key F4(132);
    const Key F5(133);
    const Key F6(134);
    const Key F7(135);
    const Key F8(136);
    const Key F9(137);
    const Key F10(138);
    const Key F11(139);
    const Key F12(141);
    const Key F13(142);
    const Key F14(143);
    const Key F15(144);
    const Key F16(145);
    const Key F17(146);
    const Key F18(147);
    const Key F19(148);
    const Key F20(149);
    const Key F21(150);
    const Key F22(151);
    const Key F23(152);
    const Key F24(153);

    const Key CapsLock(160);
    const Key NumLock(161);
    const Key ScrollLock(162);
    const Key PrintScreen(163);
    const Key Pause(164);
    const Key LeftShift(165);
    const Key RightShift(166);
    const Key LeftControl(167);
    const Key RightControl(168);
    const Key LeftAlt(169);
    const Key RightAlt(170);
    const Key LeftGui(171);
    const Key RightGui(172);
    const Key LeftMenu(173);
    const Key RightMenu(174);

    const Key Right(176);
    const Key Left(177);
    const Key Up(178);
    const Key Down(179);

    const Key PadDivide(180);
    const Key PadMultiply(181);
    const Key PadSubtract(182);
    const Key PadAdd(183);
    const Key PadEnter(184);
    const Key PadDecimal(185);
    const Key PadEquals(186);
    const Key Pad0(187);
    const Key Pad1(188);
    const Key Pad2(189);
    const Key Pad3(190);
    const Key Pad4(191);
    const Key Pad5(192);
    const Key Pad6(193);
    const Key Pad7(194);
    const Key Pad8(195);
    const Key Pad9(196);

    const Key Delete(200);
    const Key Home(201);
    const Key Insert(202);
    const Key End(203);
    const Key PageUp(204);
    const Key PageDown(205);

    // Key constants - mouse
    const int kFirstMouseKeyIndex = 300;
    const Key MouseWheelUp(kFirstMouseKeyIndex + 0);
    const Key MouseWheelDown(kFirstMouseKeyIndex + 1);
    const Key MouseLButton(kFirstMouseKeyIndex + 2);
    const Key MouseRButton(kFirstMouseKeyIndex + 3);
    const Key MouseMButton(kFirstMouseKeyIndex + 4);
    const Key Mouse4Button(kFirstMouseKeyIndex + 5);
    const Key Mouse5Button(kFirstMouseKeyIndex + 6);
    const Key Mouse6Button(kFirstMouseKeyIndex + 7);
    const Key Mouse7Button(kFirstMouseKeyIndex + 8);
    const Key Mouse8Button(kFirstMouseKeyIndex + 9);
  }

  struct KeyEvent {
    enum PressType { DOWN, REPEAT, UP };

    KeyEvent() : mouse_x(MOUSEPOS_UNKNOWN), mouse_y(MOUSEPOS_UNKNOWN), pressed(UP) { };

    int mouse_x, mouse_y;
    
    PressType pressed;

    Key key;

    std::string typed;

    // in theory these can be derived from the key event stream, but they're *so* convenient that we're just going to carry them along for now
    bool shift;
    bool ctrl;
    bool alt;

    enum { MOUSEPOS_UNKNOWN = -12345678 };
  };
}

#endif // GLOP_INPUT_H__
