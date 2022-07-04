/*
	风格设置
*/

BOOL CMyEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= AFX_WS_DEFAULT_VIEW | WS_VSCROLL | ES_AUTOVSCROLL | ES_MULTILINE | ES_NOHIDESEL;
	cs.style |= ES_READONLY;

	return bPreCreated;
    
	// return CEditView::PreCreateWindow(cs);
}


/*
	多行日志
*/

VOID CMyEditView::display_log(CString strLog)
{
	// 阻止 CEdit 控件重绘，防止界面闪烁
    GetEditCtrl().SetRedraw(FALSE);

    int nLineCount = GetEditCtrl().GetLineCount();

    // 如果输出日志超过1000行，则删除第一行
#define LINE_LIMIT 1000
    if (nLineCount > LINE_LIMIT) {
        int nBegin = 0;
        int nEnd = nBegin + GetEditCtrl().LineLength(nBegin) + _tcslen(L"\r\n");

        GetEditCtrl().SetSel(nBegin, nEnd);
        GetEditCtrl().Clear();
    }

    // 每行输出日志结尾加上换行符
    strLog += L"\r\n";

    int nTextLen = GetEditCtrl().GetWindowTextLength();
    GetEditCtrl().SetSel(nTextLen, nTextLen, FALSE);
    GetEditCtrl().ReplaceSel(strLog);
    GetEditCtrl().LineScroll(GetEditCtrl().GetLineCount());

    GetEditCtrl().SetRedraw(TRUE);

}


/* 
    背景及字体颜色，重载 WM_CTLCOLOR 消息
 */

HBRUSH CCmdDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
#define WHITE_COLOR RGB(255, 255, 255)
#define BLACK_COLOR RGB(0, 0, 0)
    if (CTLCOLOR_EDIT == nCtlColor) {
            pDC->SetBkColor(BLACK_COLOR);
            pDC->SetTextColor(WHITE_COLOR);
            return (HBRUSH)::GetStockObject(BLACK_BRUSH);
    }
    return hbr;
}