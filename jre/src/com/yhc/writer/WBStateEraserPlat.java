package com.yhc.writer;

class WBStateEraserPlat implements WBStateI {
	private WBStateEraser	_st = null;
	private WBoardPlat      _board = null;

	// set public to access dynamically through 'Class' class
	public WBStateEraserPlat(Object boardplat) {
		_board = (WBoardPlat)boardplat;
		_st = new WBStateEraser(_board.bpi());
	}

	// STate Platform Independent.
	WBStateEraser stpi() {
		return _st;
	}

	@Override
	public String name() {
		return this.getClass().getName();
	}

	@Override
	public boolean onTouch(Object o) {
		return true;
	}

	@Override
	public void draw(Object o) {

	}
}
