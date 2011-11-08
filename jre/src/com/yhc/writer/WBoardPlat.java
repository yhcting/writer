package com.yhc.writer;

import java.awt.Graphics;
import java.awt.event.ActionEvent;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.WritableRaster;
import java.io.IOException;
import java.util.LinkedList;

import javax.swing.AbstractAction;
import javax.swing.JComponent;
import javax.swing.JPanel;
import javax.swing.KeyStroke;

import com.yhc.writer.G2d.Rect;


class WBoardPlat extends JPanel implements
WBoard.Plat,
MouseListener,
MouseMotionListener,
MouseWheelListener,
ComponentListener
{
	// Not used. To make compiler be happy.
	static final long serialVersionUID = 0;

	private static final int _DIVISION_SIZE_FACTOR = 2;

	private WBoard 		_bpi; // Board Platform Independent
	private BufferedImage	_bdimg;
	private int		_wf, _hf;

	// This is uggly hack....
	// -1 : before paint
	//  0 : after paint
	//  1 : end of initialization
	private int             _bfirst_after_draw = -1;

	// AMI : Action Map Index
	private static final int _AMI_VK   = 0;
	private static final int _AMI_MASK = 1;
	private static final int _AMI_CMD  = 2;
	private static final int _AMI_ACT  = 3;
	private final Object[][] _actmap = {
			{ KeyEvent.VK_U,
				KeyEvent.CTRL_MASK,
				"undo",
				new KActUndo() },
			{ KeyEvent.VK_R,
				KeyEvent.CTRL_MASK,
				"redo",
				new KActRedo() }
	};

	private int _getX(MouseEvent e) {
		int x = e.getX();
		return (x < getWidth()) ? x : getWidth();
	}

	private int _getY(MouseEvent e) {
		int y = e.getY() - getVisibleRect().y;
		int vh = (int) getVisibleRect().getHeight();
		return (y < vh) ? y : vh;
	}

	// ==================================
	// Implements MouseListener
	// ==================================
	public void mousePressed(MouseEvent e) {
		_bpi.onTouch(e);
	}

	public void mouseReleased(MouseEvent e) {
		_bpi.onTouch(e);
	}


	public void mouseEntered(MouseEvent e) {

	}

	public void mouseClicked(MouseEvent e) {

	}

	public void mouseExited(MouseEvent e) {

	}

	// ==================================
	// Implements MouseMotionListener
	// ==================================
	public void mouseDragged(MouseEvent e) {
		_bpi.onTouch(e);
	}

	public void mouseMoved(MouseEvent e) {
	}

	// ==================================
	// Implements MouseWheelListener
	// ==================================
	public void mouseWheelMoved(MouseWheelEvent e) {
		_bpi.onTouch(e);
	}

	// ==================================
	// Implements Key Actions
	// ==================================
	private class KActUndo extends AbstractAction {
		// Not used. To make compiler be happy.
		static final long serialVersionUID = 0;
		public void actionPerformed(ActionEvent ev) {
			_bpi.undo();
			// redraw whole sheet again.
			_bpi.cleanDrawSheet();
			invalidatePlatBoard();
		}
	}

	private class KActRedo extends AbstractAction {
		// Not used. To make compiler be happy.
		static final long serialVersionUID = 0;
		public void actionPerformed(ActionEvent ev) {
			_bpi.redo();
			// redraw whole sheet again.
			_bpi.cleanDrawSheet();
			invalidatePlatBoard();
		}
	}

	private void _addKeyBindings(JComponent comp) {
		// Frame Key Bindings
		for (Object[] e : _actmap) {
			comp.getInputMap()
				.put(KeyStroke.getKeyStroke(
					(Integer) e[_AMI_VK],
					(Integer) e[_AMI_MASK]),
					e[_AMI_CMD]);
			comp.getActionMap().put(
				(String) e[_AMI_CMD],
				(AbstractAction) e[_AMI_ACT]);
		}
	}

	// ==================================
	// Implements ComponentListener
	// ==================================
	public void componentMoved(ComponentEvent e) {

	}

	public void componentHidden(ComponentEvent e) {

	}

	public void componentResized(ComponentEvent e) {
		if (0 == _bfirst_after_draw) {
			_bfirst_after_draw = 1;
			_bpi.createNew(getWidth() / _DIVISION_SIZE_FACTOR,
					getHeight() / _DIVISION_SIZE_FACTOR,
					_wf * _DIVISION_SIZE_FACTOR,
					_hf * _DIVISION_SIZE_FACTOR);
			_bpi.initScreen(getWidth(), getHeight());

			ColorModel cm = ColorModel.getRGBdefault();
			SampleModel sm = cm.createCompatibleSampleModel(_bpi.sw(), _bpi.sh());
			DataBuffer db = new DataBufferInt(_bpi.pixels(), _bpi.pixels().length);
			WritableRaster wr = Raster.createWritableRaster(sm, db, null);
			_bdimg = new BufferedImage(cm, wr, false, null);

			_bindListeners();
		}
	}

	public void componentShown(ComponentEvent e) {
	}

	private void _bindListeners() {
		addMouseListener(this);
		addMouseMotionListener(this);
		addMouseWheelListener(this);
		_addKeyBindings(this);
	}

	@Override
	public void paintComponent(Graphics g) {
		if (_bfirst_after_draw < 0)
			_bfirst_after_draw = 0;

		if (_bfirst_after_draw > 0) {
			g.drawImage(_bdimg, 0, 0, null);
			_bpi.onDraw(g);
		}
	}
	/**************************
	 * Implements interfaces.
	 **************************/
	public void invalidatePlatBoard() {
		repaint(getBounds());
	}

	public void invalidatePlatBoard(Rect r) {
		repaint(WAL.convertFrom(r));
	}

	public void invalidateBoard(int left, int top, int right, int bottom) {
		_bpi.invalidateBoard(left, top, right, bottom);
	}
	/**************************
	 * APIs.
	 **************************/
	WBoardPlat(int w, int h) {
		super();
		_bpi = new WBoard();
		_bpi.init(this, WBStatePenPlat.class.getName(), WBStateEraserPlat.class.getName());


		WBStatePenPlat pen = (WBStatePenPlat)_bpi.penState();
		pen.setEraser(((WBStateEraserPlat)_bpi.eraserState()).stpi());

		_wf = w;
		_hf = h;

		addComponentListener(this);
	}

	void destroy() {
		_bpi.destroy();
	}

	WBoard bpi() {
		return _bpi;
	}

	int sheetWidth() {
		return _bpi.sheetWidth();
	}

	int sheetHeight() {
		return _bpi.sheetHeight();
	}

	int bgcolor() {
		return _bpi.bgcolor();
	}

	boolean zoom(float zov) {
		return _bpi.zoom(zov);
	}

	WBStateI state() {
		return _bpi.state();
	}

	void initUiState() {
		_bpi.initUiState();
	}

	void setState(Object trigger_owner, String statecls) {
		_bpi.setState(trigger_owner, statecls);
	}

	void moveActiveRegion(Object trigger_owner, int dx, int dy) {
		_bpi.moveActiveRegion(trigger_owner, dx, dy);
	}

	void moveActiveRegionTo(Object trigger_owner, int left, int top) {
		_bpi.moveActiveRegionTo(trigger_owner, left, top);
	}

	void updateCurve(LinkedList<G2d.Point> points, byte thick, int color) {
		_bpi.updateCurve(points, thick, color);
	}

	void drawLine(int x0, int y0, int x1, int y1, byte thick, int color) {
		_bpi.drawLine(x0, y0, x1, y1, thick, color);
	}

	void fill(int left, int top, int right, int bottom, int bgcolor, byte thick){
		_bpi.fill(left, top, right, bottom, bgcolor, thick);
	}

	void drawSheet(int left, int top, int right, int bottom) {
		_bpi.drawSheet(left, top, right, bottom);
	}

	void cutoutSheet(int left, int top, int right, int bottom) {
		_bpi.cutoutSheet(left, top, right, bottom);
	}

	void saveSheet(String fpath) throws IOException {
		_bpi.saveSheet(fpath);
	}

	void loadSheet(String fpath) throws IOException {
		_bpi.loadSheet(fpath);
	}
}
