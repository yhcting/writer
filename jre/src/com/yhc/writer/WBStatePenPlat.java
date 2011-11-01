package com.yhc.writer;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;

import com.yhc.writer.G2d.Rect;


// TODO
//   I think we can use button2 for eraser....
//   How can I integrate pen and eraser to one instance!
//
class WBStatePenPlat implements WBStateI {
	private WBStatePen	_st = null;
	private WBStateEraser   _ste = null; // eraser...
	private WBoardPlat      _board = null;

	private int		_btn_captured = MouseEvent.NOBUTTON;
	private int             _x, _y;

	private void onMouse_wheel(MouseWheelEvent e) {
		// 5 scroll for zoom in/out by 2-times
		_st.onActionZoom((e.getUnitsToScroll() > 0)? 1.2f: 0.8f);
	}

	private void onMouse1_press(int x, int y) {
		_st.onActionStart();
	}

	private void onMouse1_drag(int x0, int y0, int x1, int y1) {
		_st.onActionLine(x0, y0, x1, y1);
	}

	private void onMouse1_release(int x, int y) {
		_st.onActionEnd();
	}

	private void onMouse2_press(int x, int y) {
		_ste.onActionStart(x, y);
	}

	private void onMouse2_drag(int x0, int y0, int x1, int y1) {
		_ste.onActionMove(x1, y1);
	}

	private void onMouse2_release(int x, int y) {
		_ste.onActionEnd();
	}

	private void onMouse3_press(int x, int y) {
		_st.onActionStart();
	}

	private void onMouse3_drag(int x0, int y0, int x1, int y1) {
		_st.onActionMove(x1 - x0, y1 - y0);
	}

	private void onMouse3_release(int x, int y) {
		_st.onActionEnd();
	}

	// set public to access dynamically through 'Class' class
	public WBStatePenPlat(Object boardplat) {
		_board = (WBoardPlat)boardplat;
		_st = new WBStatePen(_board.bpi());
	}

	void setEraser(WBStateEraser eraser) {
		_ste = eraser;
	}

	@Override
	public String name() {
		return this.getClass().getName();
	}

	@Override
	public boolean onTouch(Object o) {
		// BIG-RULE : one-event at one time.
		if (o instanceof java.awt.event.MouseWheelEvent) {
			// if there is captured button ignore wheel(zoom) event.
			if (MouseEvent.NOBUTTON == _btn_captured)
				onMouse_wheel((MouseWheelEvent)o);
		} else {
			MouseEvent e = (MouseEvent)o;
			switch (e.getID()) {
			case MouseEvent.MOUSE_PRESSED: {
				if (MouseEvent.NOBUTTON == _btn_captured) {
					int btn = e.getButton();
					_btn_captured = btn;
					_x = e.getX();
					_y = e.getY();
					switch (btn) {
					case MouseEvent.BUTTON1:
						onMouse1_press(_x, _y);
					break;
					case MouseEvent.BUTTON2:
						onMouse2_press(_x, _y);
					break;
					case MouseEvent.BUTTON3: // button3 is right button
						onMouse3_press(_x, _y);
					break;
					default:
						// ignore others.
						_btn_captured = MouseEvent.NOBUTTON;
					}

				}
				// Ignore other press event if button is already captured!
			} break;
			case MouseEvent.MOUSE_DRAGGED: {
				if (MouseEvent.NOBUTTON != _btn_captured) {
					int mask = e.getModifiersEx();
					int x = e.getX();
					int y = e.getY();
					switch (_btn_captured) {
					case MouseEvent.BUTTON1:
						if (0 != (mask & MouseEvent.BUTTON1_DOWN_MASK)) {
							onMouse1_drag(_x, _y, x, y);
							_x = x;
							_y = y;
						}
					break;
					case MouseEvent.BUTTON2:
						if (0 != (mask & MouseEvent.BUTTON2_DOWN_MASK)) {
							onMouse2_drag(_x, _y, x, y);
							_x = x;
							_y = y;
						}
					case MouseEvent.BUTTON3:
						if (0 != (mask & MouseEvent.BUTTON3_DOWN_MASK)) {
							onMouse3_drag(_x, _y, x, y);
							_x = x;
							_y = y;
						}
					break;
					default:
						// Should NOT reach here!
						WDev.wassert(false);
					}
				}
			} break;
			case MouseEvent.MOUSE_RELEASED: {
				if (MouseEvent.NOBUTTON != _btn_captured && e.getButton() == _btn_captured) {
					int x = e.getX();
					int y = e.getY();
					switch (_btn_captured) {
					case MouseEvent.BUTTON1:
						onMouse1_release(x, y);
					break;
					case MouseEvent.BUTTON2:
						onMouse2_release(x, y);
					break;
					case MouseEvent.BUTTON3:
						onMouse3_release(x, y);
					break;
					default:
						// Should NOT reach here
						WDev.wassert(false);
					}
					_btn_captured = MouseEvent.NOBUTTON;
				}
			} break;
			default:
				WDev.wassert(false);
			}
		}

		return true;
	}

	@Override
	public void draw(Object o) {
		if (MouseEvent.BUTTON2 == _btn_captured) {
			// This is eraser state...
			Graphics g = (Graphics)o;
			if (!_ste.track().isEmpty()) {
				Rect r = _ste.track();
				g.setXORMode(Color.white);
				g.drawRect(r.l, r.t, r.width(), r.height());
			}
		}
	}
}
