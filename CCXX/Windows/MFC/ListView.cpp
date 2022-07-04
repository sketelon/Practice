
/*
# 目录：
1. 自动调整宽度适应 column 的宽度
2. column 等宽
3. 设置高度（取巧）
4. 遍历进程并插入
5. 设置风格
6. 弹出菜单
7. 设置系统图标
8. 清空全部列和行
*/




/*
	1. 自动调整宽度适应 column 的宽度
*/

void CXXXListView::adjust_column_width()
{
	SetRedraw(FALSE);
	int nColumnCount = GetListCtrl().GetHeaderCtrl()->GetItemCount();

	for (int i = 0; i < nColumnCount; i++)
	{
		GetListCtrl().SetColumnWidth(i, LVSCW_AUTOSIZE);
		int nColumnWidth = GetListCtrl().GetColumnWidth(i);
		GetListCtrl().SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		int nHeaderWidth = GetListCtrl().GetColumnWidth(i);
		GetListCtrl().SetColumnWidth(i, max(nColumnWidth, nHeaderWidth));
	}
	SetRedraw(TRUE);
}

/*
	2. column 等宽，不要放在 OnCreate 中，可以放在 OnInitialUpdate 中
*/

void CXXXListView::equal_column_width()
{
	SetRedraw(FALSE);
	CRect rc;
	int nColumnCount = GetListCtrl().GetHeaderCtrl()->GetItemCount();
	GetListCtrl().GetClientRect(&rc);
	for (int i = 0; i < nColumnCount; i++)
	{
		GetListCtrl().SetColumnWidth(i, rc.Width() / nColumnCount);
	}
	SetRedraw(TRUE);
}


/*
	3. 设置高度（取巧）
*/

void CXXXListView::set_row_heigt(int nHeight)
{
     CImageList im;
     im.Create(1, nHeight, ILC_COLOR4, 10, 10);
	 GetListCtrl().SetImageList(&im, LVSIL_SMALL);
 }

 /*
	4. 遍历进程并插入
 */

void CXXXListView::insert_process()
{
	int nColumn = 0;
	GetListCtrl().InsertColumn(nColumn++, _T("Name"), LVCFMT_CENTER);
	GetListCtrl().InsertColumn(nColumn++, _T("PID"), LVCFMT_CENTER);

	int nRow = 0;
	PROCESSENTRY32 pe32 = { 0 };

	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		goto EXIT;
	}

	do
	{
		WCHAR szBuff[_MAX_ITOSTR_BASE10_COUNT] = { 0 };
		_itow_s(pe32.th32ProcessID, szBuff, 10);

		int nSubItem = 1;
		GetListCtrl().InsertItem(nRow, pe32.szExeFile);
		GetListCtrl().SetItemText(nRow, nSubItem++, szBuff);
		nRow++;

	} while (Process32Next(hProcessSnap, &pe32));

EXIT:
	CloseHandle(hProcessSnap);
}

/*
	5. 设置风格
*/

void CXXXListView::set_style()
{
	CListCtrl& listCtrl = GetListCtrl();

	LONG lStyle;
	lStyle = GetWindowLong(m_hWnd, GWL_STYLE);	//获取当前窗口style      
	lStyle &= ~LVS_TYPEMASK;					//清除显示方式位     
	lStyle |= LVS_REPORT;					//设置style     
	SetWindowLong(m_hWnd, GWL_STYLE, lStyle);	//设置style   

	DWORD dwStyle = listCtrl.GetExtendedStyle();
	dwStyle |= LVS_EX_FULLROWSELECT;	//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;		//网格线（只适用与report风格的listctrl）  
	//dwStyle |= LVS_EX_CHECKBOXES;		//item前生成checkbox控件     
	listCtrl.SetExtendedStyle(dwStyle);
}


 /*
	6. 弹出菜单
 */

 void CXXXListView::OnNMRClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here

	//获取鼠标坐标
	CPoint ptCurSel(0, 0);
	GetCursorPos(&ptCurSel);
	//转换为当前控件坐标系的坐标
	GetListCtrl().ScreenToClient(&ptCurSel);
	LVHITTESTINFO HitTestInfo;
	HitTestInfo.pt = ptCurSel;
	//判断坐标命中第几项
	int nItem = GetListCtrl().HitTest(&HitTestInfo);

	//如果有选中项
	if (nItem != -1)
	{

		//弹出菜单
		CMenu menu;
		menu.LoadMenu(IDR_MENU1);
		CPoint point;
		GetCursorPos(&point);
		CMenu* pSubMenu = menu.GetSubMenu(0);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN, point.x, point.y, this);
	}

	*pResult = 0;
}


void CXXXListView::OnPopPop()
{
	// TODO: Add your command handler code here
	int nItemCount = GetListCtrl().GetSelectionMark();
	WCHAR szBuff[_MAX_ITOSTR_BASE10_COUNT] = { 0 };
	_itow_s(nItemCount, szBuff, 10);

	AfxMessageBox(szBuff);
}


 /*
	7. 设置系统图标
 */


void CXXXListView::initialize_icon()
{

	HIMAGELIST hImageListLarge = NULL;
	HIMAGELIST hImageListSmall = NULL;
	Shell_GetImageLists(&hImageListLarge, &hImageListSmall);
	ListView_SetImageList(GetListCtrl().m_hWnd, hImageListLarge, LVSIL_NORMAL);
	ListView_SetImageList(GetListCtrl().m_hWnd, hImageListSmall, LVSIL_SMALL);
}

int get_icon_index(LPCTSTR lpFileName, DWORD dwFileAttributes)
{
	SHFILEINFO	sfi;

	dwFileAttributes == INVALID_FILE_ATTRIBUTES ?
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL : 
		dwFileAttributes |= FILE_ATTRIBUTE_NORMAL;

	SHGetFileInfo
	(
		lpFileName,
		dwFileAttributes,
		&sfi,
		sizeof(SHFILEINFO),
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES
	);
	DestroyIcon(sfi.hIcon);
	return sfi.iIcon;
}

// 获得硬盘图标
int get_driver_icon(TCHAR cDriver, UINT uDriverType)
{
	int nIconIndex = -1;
	if (cDriver == 'A' || cDriver == 'B')
	{
		nIconIndex = 6;
	}
	else
	{
		switch (uDriverType)
		{
		case DRIVE_REMOVABLE:
			nIconIndex = 7;
			break;
		case DRIVE_FIXED:
			nIconIndex = 8;
			break;
		case DRIVE_REMOTE:
			nIconIndex = 9;
			break;
		case DRIVE_CDROM:
			nIconIndex = 11;
			break;
		default:
			nIconIndex = 8;
			break;
		}
	}

	return nIconIndex;
}

/*
	8. 清空全部列和行
*/
void CXXXListView::delete_all_items()
{
	DeleteAllItems();
	while(GetListCtrl().DeleteColumn(0));
}
