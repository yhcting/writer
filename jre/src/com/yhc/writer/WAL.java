package com.yhc.writer;

import java.awt.Rectangle;

class WAL implements WPlatform.IAL {
	static WAL _instance = null;

	WAL() {
	}

	static WAL instance() {
		if (null == _instance)
			_instance = new WAL();

		return _instance;
	}

	static Rectangle convertFrom(G2d.Rect r) {
		return new Rectangle(r.l, r.t, r.r, r.b);
	}

	static G2d.Rect convertTo(Rectangle r) {
		return new G2d.Rect(r.x, r.y, r.x + r.width, r.y + r.height);
	}

	public void log(String prefix, String msg) {
		System.out.println("[Writer]" + prefix + msg);
	}
}
