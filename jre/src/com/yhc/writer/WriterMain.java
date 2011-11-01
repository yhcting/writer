package com.yhc.writer;

import javax.swing.JFrame;


class WriterMain extends JFrame {
	private static final int _FRM_WIDTH	= 1024;
	private static final int _FRM_HEIGHT	= 768;

	WriterMain(String title) {
		super(title);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setSize(_FRM_WIDTH, _FRM_HEIGHT);
		WPlatform.setAL(new WAL());
		add(new WBoardPlat(10, 10));
		// pack();
		setVisible(true);
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		new WriterMain("Writer-Java");
	}

}
