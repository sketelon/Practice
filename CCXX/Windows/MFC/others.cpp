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




/*
视图切割
1. 重写 CFrameWnd 类成员虚函数 OnCreateClient
2. 在虚函数中调用CSplitterWnd::CreateStatic 创建不规则框架窗口
3. 在虚函数中调用CSplitterWnd::CreateView 创建视图窗口

*/

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	// TODO: Add your specialized code here and/or call the base class
	m_SplitterWnd.CreateStatic(this, 2, 1);

	CRect rc;
	GetClientRect(&rc);

	m_SplitterWnd.CreateView(0, 0, RUNTIME_CLASS(CMainTabView), 
		CSize(rc.Width(), rc.Height() / 5 * 3), pContext);
	m_SplitterWnd.CreateView(1, 0, RUNTIME_CLASS(CMyEditView), 
		CSize(rc.Width(), rc.Height() / 5 * 2), pContext);

	// return CFrameWnd::OnCreateClient(lpcs, pContext);
	// 注意此处应应修改为 return TRUE;
	return TRUE;
}