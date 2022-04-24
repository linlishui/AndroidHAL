package com.android.commands.vireg;

import android.os.ServiceManager;
import android.os.IViregService;
import android.util.AndroidException;

import com.android.internal.os.BaseCommand;

import java.io.PrintStream;
import java.io.PrintWriter;

public class Vireg extends BaseCommand {

	private static final String SHELL_PACKAGE_NAME = "com.android.shell";

	private IViregService mVs;

	/**
	* Command-line entry point.
	*
	* @param args The command-line arguments
	*/
	public static void main(String[] args) {
		(new Vireg()).run(args);
	}

	@Override
	public void onRun() throws Exception {
		mVs = IViregService.Stub.asInterface(ServiceManager.getService("vireg"));
		if (mVs == null) {
			System.err.println(NO_SYSTEM_ERROR_CODE);
			throw new AndroidException("Can't connect to vireg service; is the system running?");
		}

		String op = nextArgRequired();

		if ("read".equals(op)) {
			runRead();
		} else if ("write".equals(op)) {
			runWrite();
		}
	}

	@Override
	public void onShowUsage(PrintStream out) {
		PrintWriter pw = new PrintWriter(out);
		pw.println(
				"vireg [read/write]"
		);
	}

	private void runRead() {
		int value = mVs.getVal();
		System.out.println("vireg current value = " + value);
	}

	private void runWrite() {
		String value = nextArgRequired();
		try {
			mVs.setVal(Integer.parseInt(value));
			System.out.println("vireg set value success with " + value);
		} catch (Exception ex) {
			System.err.println("vireg set value fail with " + value);
		}
	}
}