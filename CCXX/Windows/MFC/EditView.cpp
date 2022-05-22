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
