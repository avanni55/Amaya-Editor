#ifdef _WX

#include "wx/wx.h"
#include "wx/tipdlg.h"
#include "wx/artprov.h"


#include "thot_gui.h"
#include "thot_sys.h"
#include "constmedia.h"
#include "typemedia.h"
#include "appdialogue.h"
#include "dialog.h"
#include "selection.h"
#include "application.h"
#include "dialog.h"
#include "document.h"
#include "message.h"
#include "message_wx.h"
#include "libmsg.h"
#include "frame.h"
#include "logdebug.h"

#undef THOT_EXPORT
#define THOT_EXPORT extern


#include "AmayaTipOfTheDay.h"
#include "tips.h"

static const int s_tips[] = 
{
    LIB, TIP_AMAYA_PROFILES,
    LIB, TIP_PANEL_MODES,
    LIB, TIP_PARENT_SELECTION,
    LIB, TIP_SPLIT_NOTEBOOK,
    LIB, TIP_MOVE_PANELS,
    LIB, TIP_MANY_COLUMNS,
    LIB, TIP_EDITING_MODE,
    LIB, TIP_ERROR_BUTTON,
    LIB, TIP_PATH_CONTROL,
    LIB, TIP_DOCUMENT_LIST_MENU
};

//  TtaGetEnvInt

static const int s_tipsCount = sizeof(s_tips)/sizeof(int)/2;



/*----------------------------------------------------------------------
 -----------------------------------------------------------------------*/
AmayaTipProvider::AmayaTipProvider(size_t currentTip):
  wxTipProvider(currentTip)
{
}

/*----------------------------------------------------------------------
 -----------------------------------------------------------------------*/
AmayaTipProvider::~AmayaTipProvider()
{
}

/*----------------------------------------------------------------------
 -----------------------------------------------------------------------*/
wxString AmayaTipProvider::GetTip()
{
  int num = m_currentTip++;
   m_currentTip %= s_tipsCount;

  return TtaConvMessageToWX(TtaGetMessage(s_tips[num*2],
                                          s_tips[num*2+1]));
}



// ----------------------------------------------------------------------------
// wxTipDialog
// ----------------------------------------------------------------------------

static const int wxID_NEXT_TIP = 32000;  // whatever

BEGIN_EVENT_TABLE(AmayaTipDialog, wxDialog)
    EVT_BUTTON(wxID_NEXT_TIP, AmayaTipDialog::OnNextTip)
END_EVENT_TABLE()

/*----------------------------------------------------------------------
 -----------------------------------------------------------------------*/
AmayaTipDialog::AmayaTipDialog(wxWindow *parent,
                         wxTipProvider *tipProvider,
                         bool showAtStartup)
           : wxDialog(parent, wxID_ANY, TtaConvMessageToWX(TtaGetMessage(LIB, TIP_DIALOG_TITLE)),
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
                      )
{
    m_tipProvider = tipProvider;

    // 1) create all controls in tab order
    wxButton *btnClose = new wxButton(this, wxID_CLOSE);
    SetAffirmativeId(wxID_CLOSE);

    m_checkbox = new wxCheckBox(this, wxID_ANY, TtaConvMessageToWX(TtaGetMessage(LIB, TIP_DIALOG_STARTUP)));
    m_checkbox->SetValue(showAtStartup);

    wxButton *btnNext = new wxButton(this, wxID_NEXT_TIP, TtaConvMessageToWX(TtaGetMessage(LIB, TIP_DIALOG_NEXT_TIP)));

    wxStaticText *text = new wxStaticText(this, wxID_ANY, TtaConvMessageToWX(TtaGetMessage(LIB, TIP_DIALOG_DID_YOU_KNOW)));

    wxFont font = text->GetFont();
    font.SetPointSize(int(1.6 * font.GetPointSize()));
    font.SetWeight(wxFONTWEIGHT_BOLD);
    text->SetFont(font);

    m_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                            wxDefaultPosition, wxSize(200, 160),
                            wxTE_MULTILINE |
                            wxTE_READONLY |
                            wxTE_NO_VSCROLL |
                            wxTE_RICH2 | // a hack to get rid of vert scrollbar
                            wxDEFAULT_CONTROL_BORDER
                            );
#if defined(__WXMSW__)
    m_text->SetFont(wxFont(12, wxSWISS, wxNORMAL, wxNORMAL));
#endif

    wxIcon icon = wxArtProvider::GetIcon(wxART_TIP, wxART_CMN_DIALOG);
    wxStaticBitmap *bmp = new wxStaticBitmap(this, wxID_ANY, icon);

    // 2) put them in boxes

    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );
    icon_text->Add( bmp, 0, wxCENTER );
    icon_text->Add( text, 1, wxCENTER | wxLEFT, 20 );
    topsizer->Add( icon_text, 0, wxEXPAND | wxALL, 10 );

    topsizer->Add( m_text, 1, wxEXPAND | wxLEFT|wxRIGHT, 10 );

    wxBoxSizer *bottom = new wxBoxSizer( wxHORIZONTAL );
    bottom->Add( m_checkbox, 0, wxCENTER );

    bottom->Add( 10,10,1 );
    bottom->Add( btnNext, 0, wxCENTER | wxLEFT, 10 );
    bottom->Add( btnClose, 0, wxCENTER | wxLEFT, 10 );

    topsizer->Add( bottom, 0, wxEXPAND | wxALL, 10 );

    SetTipText();

    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre(wxBOTH | wxCENTER_FRAME);
}

/*----------------------------------------------------------------------
 -----------------------------------------------------------------------*/
static bool AmayaShowTip(wxWindow *parent,
               wxTipProvider *tipProvider,
               bool showAtStartup)
{
  AmayaTipDialog dlg(parent, tipProvider, showAtStartup);
  dlg.ShowModal();

  return dlg.ShowTipsOnStartup();
}



#endif /* #ifdef _WX */


/*----------------------------------------------------------------------
 * Show TipOfTheDay dialog
 -----------------------------------------------------------------------*/
void TtaShowTipOfTheDay()
{
#ifdef _WX
  int num = 0;
  ThotBool res, show = TtaShowTipOfTheDayAtStartup();
  
  TtaGetEnvInt("TIP_OF_THE_DAY_NUMBER", &num);
  
  AmayaTipProvider prov(num);
  res = AmayaShowTip(NULL,& prov, show);
  
  if(res!=show)
    TtaSetShowTipOfTheDayAtStartup(res);
  TtaSetEnvInt("TIP_OF_THE_DAY_NUMBER", (int)prov.GetCurrentTip(), TRUE);
#endif /* _WX */
}

/*----------------------------------------------------------------------
 * Test if TipOfTheDay must be shown at startup
 -----------------------------------------------------------------------*/
ThotBool TtaShowTipOfTheDayAtStartup()
{
  ThotBool show = TRUE;
  TtaGetEnvBoolean("TIP_OF_THE_DAY_STARTUP", &show);
  return show;
}

/*----------------------------------------------------------------------
 * Set if TipOfTheDay must be shown at startup
 -----------------------------------------------------------------------*/
void TtaSetShowTipOfTheDayAtStartup(ThotBool show)
{
  TtaSetEnvBoolean("TIP_OF_THE_DAY_STARTUP", show, TRUE);
}