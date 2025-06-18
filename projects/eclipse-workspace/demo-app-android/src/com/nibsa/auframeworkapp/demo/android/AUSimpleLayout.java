package com.serenehearts.android; //

import android.content.Context;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

public class AUSimpleLayout extends ViewGroup {
	
	public AUSimpleLayout(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
	}

	@Override
	protected void onLayout(boolean changed, int l, int t, int r, int b) {
		// TODO Auto-generated method stub
		final int count = getChildCount();
		int curWidth, curHeight;//, curLeft, curTop, maxHeight;

		//get the available size of child view    
		int childLeft = this.getPaddingLeft();
		int childTop = this.getPaddingTop();
		int childRight = this.getMeasuredWidth() - this.getPaddingRight();
		int childBottom = this.getMeasuredHeight() - this.getPaddingBottom();
		int childWidth = childRight - childLeft;
		int childHeight = childBottom - childTop;
		Log.i("AU", "child limits l(" + childLeft + ") r(" + childRight + ") t("+childTop+") b("+childBottom+")");
		if(count > 0){
			//First item: full layout
			View firstChild = getChildAt(0);
			firstChild.layout(childLeft, childTop, childRight, childBottom);
			Log.i("AU", "firstChild w(" + (childRight - childLeft) + ") h(" + (childTop - childBottom) + ")");
			//walk through each child, and arrange it from left to right
			int curBottom = childBottom;
			for (int i = 1; i < count; i++) {
				View child = getChildAt(i);
				if (child.getVisibility() != GONE) {
					//Get the maximum size of the child
					child.measure(MeasureSpec.makeMeasureSpec(childWidth, MeasureSpec.AT_MOST), MeasureSpec.makeMeasureSpec(childHeight, MeasureSpec.AT_MOST));
					curWidth = child.getMeasuredWidth();
					curHeight = child.getMeasuredHeight();
					//do the layout
					int leftDst = childLeft + ((childWidth - curWidth) / 2);
					child.layout(leftDst, curBottom - curHeight, leftDst + curWidth, curBottom);
					//store the max height
					curBottom -= curHeight;
					Log.i("AU", "child w(" + curWidth + ") h(" + curWidth + ") curBottom(" + curBottom + ")");
				}
			}
		}
	}
}
