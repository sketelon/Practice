/*

	MFC 自定义消息
	1. #define  WM_MYMSG (WM_USER + 1)
	2. afx_msg LRESULT OnMyMessage(WPARAM wParam, LPARAM lParam);
	3. 消息映射里添加：ON_MESSAGE(WM_MYMSG, OnMyMessage)


	关于 PostMessage 中窗口句柄获得：
	1. 可以在窗口类中直接定义一个**静态成员**:static CWnd* m_pWnd;
	2. 在OnCreate（也可以在其他地方）中m_pWnd = this
	3. CXXXView::pMyListWnd->GetSafeHwnd() 获得窗口句柄
	
*/