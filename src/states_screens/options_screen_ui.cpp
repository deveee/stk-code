//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "states_screens/options_screen_ui.hpp"

#include "addons/news_manager.hpp"
#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "config/hardware_stats.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "font/bold_face.hpp"
#include "font/font_manager.hpp"
#include "font/regular_face.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "online/request_manager.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/options_screen_audio.hpp"
#include "states_screens/options_screen_input.hpp"
#include "states_screens/options_screen_video.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/user_screen.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace GUIEngine;
using namespace Online;

DEFINE_SCREEN_SINGLETON( OptionsScreenUI );

// -----------------------------------------------------------------------------

OptionsScreenUI::OptionsScreenUI() : Screen("options_ui.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// -----------------------------------------------------------------------------

void OptionsScreenUI::loadedFromFile()
{
    m_inited = false;

}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenUI::init()
{
    Screen::init();
    
    // ---- music volume
    CheckBoxWidget* sfx = this->getWidget<CheckBoxWidget>("sound_effects");

    CheckBoxWidget* music = this->getWidget<CheckBoxWidget>("music");

    // ---- audio enables/disables
    sfx->setState( UserConfigParams::m_sfx );
    music->setState( UserConfigParams::m_music );
    

    CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("touch_device");
    assert(buttons_en != NULL);
    buttons_en->setState(UserConfigParams::m_multitouch_mode != 0);
    //~ UserConfigParams::m_multitouch_mode = buttons_en->getState() ? 1 : 0;

    // --- language
    ListWidget* list_widget = getWidget<ListWidget>("language");

    // I18N: in the language choice, to select the same language as the OS
    list_widget->addItem("system", _("System Language"));

    const std::vector<std::string>* lang_list = translations->getLanguageList();
    const int amount = (int)lang_list->size();

    // The names need to be sorted alphabetically. Store the 2-letter
    // language names in a mapping, to be able to get them from the
    // user visible full name.
    std::vector<core::stringw> nice_lang_list;
    std::map<core::stringw, std::string> nice_name_2_id;
    for (int n=0; n<amount; n++)
    {
        std::string code_name = (*lang_list)[n];
        std::string s_name = translations->getLocalizedName(code_name) +
         " (" + tinygettext::Language::from_name(code_name).get_language() + ")";
        core::stringw nice_name = translations->fribidize(StringUtils::utf8ToWide(s_name));
        nice_lang_list.push_back(nice_name);
        nice_name_2_id[nice_name] = code_name;
    }
    std::sort(nice_lang_list.begin(), nice_lang_list.end());
    for(unsigned int i=0; i<nice_lang_list.size(); i++)
    {
        list_widget->addItem(nice_name_2_id[nice_lang_list[i]],
                              nice_lang_list[i]);
    }

    list_widget->setSelectionID( list_widget->getItemID(UserConfigParams::m_language) );

    // Forbid changing language while in-game, since this crashes (changing the language involves
    // tearing down and rebuilding the menu stack. not good when in-game)
    list_widget->setActive(StateManager::get()->getGameState() != GUIEngine::INGAME_MENU);

}   // init

// -----------------------------------------------------------------------------

void OptionsScreenUI::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        else if (selection == "tab_controls")
            screen = OptionsScreenInput::getInstance();
        //else if (selection == "tab_ui")
        //    screen = OptionsScreenUI::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "skinchoice")
    {
        GUIEngine::SpinnerWidget* skinSelector = getWidget<GUIEngine::SpinnerWidget>("skinchoice");
        assert( skinSelector != NULL );

        const core::stringw selectedSkin = skinSelector->getStringValue();
        UserConfigParams::m_skin_file = core::stringc(selectedSkin.c_str()).c_str() + std::string(".stkskin");
        GUIEngine::reloadSkin();
    }
    else if (name == "showfps")
    {
        CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
        assert( fps != NULL );
        UserConfigParams::m_display_fps = fps->getState();
    }
    else if (name=="enable-internet")
    {
        CheckBoxWidget* internet = getWidget<CheckBoxWidget>("enable-internet");
        assert( internet != NULL );
        UserConfigParams::m_internet_status =
            internet->getState() ? RequestManager::IPERM_ALLOWED
                                 : RequestManager::IPERM_NOT_ALLOWED;
        // If internet gets enabled, re-initialise the addon manager (which
        // happens in a separate thread) so that news.xml etc can be
        // downloaded if necessary.
        CheckBoxWidget *stats = getWidget<CheckBoxWidget>("enable-hw-report");
        LabelWidget *stats_label = getWidget<LabelWidget>("label-hw-report");
        if(internet->getState())
        {
            NewsManager::get()->init(false);
            stats->setVisible(true);
            stats_label->setVisible(true);
            stats->setState(UserConfigParams::m_hw_report_enable);
        }
        else
        {
            stats->setVisible(false);
            stats_label->setVisible(false);
            PlayerProfile* profile = PlayerManager::getCurrentPlayer();
            if (profile != NULL && profile->isLoggedIn())
                profile->requestSignOut();
        }
    }
    else if (name=="enable-hw-report")
    {
        CheckBoxWidget* stats = getWidget<CheckBoxWidget>("enable-hw-report");
        UserConfigParams::m_hw_report_enable = stats->getState();
        if(stats->getState())
            HardwareStats::reportHardwareStats();
    }
    else if (name=="show-login")
    {
        CheckBoxWidget* show_login = getWidget<CheckBoxWidget>("show-login");
        assert( show_login != NULL );
        UserConfigParams::m_always_show_login_screen = show_login->getState();
    }
    else if (name=="perPlayerDifficulty")
    {
        CheckBoxWidget* difficulty = getWidget<CheckBoxWidget>("perPlayerDifficulty");
        assert( difficulty != NULL );
        UserConfigParams::m_per_player_difficulty = difficulty->getState();
    }
    else if (name == "language")
    {
        ListWidget* list_widget = getWidget<ListWidget>("language");
        std::string selection = list_widget->getSelectionInternalName();

        delete translations;

        if (selection == "system")
        {
#ifdef WIN32
            _putenv("LANGUAGE=");
#else
            unsetenv("LANGUAGE");
#endif
        }
        else
        {
#ifdef WIN32
            std::string s=std::string("LANGUAGE=")+selection.c_str();
            _putenv(s.c_str());
#else
            setenv("LANGUAGE", selection.c_str(), 1);
#endif
        }

        translations = new Translations();

        // Reload fonts for new translation
        GUIEngine::getStateManager()->hardResetAndGoToScreen<MainMenuScreen>();

        font_manager->getFont<BoldFace>()->reset();
        font_manager->getFont<RegularFace>()->reset();
        GUIEngine::getFont()->updateRTL();
        GUIEngine::getTitleFont()->updateRTL();
        GUIEngine::getSmallFont()->updateRTL();
        GUIEngine::getLargeFont()->updateRTL();
        GUIEngine::getOutlineFont()->updateRTL();

        UserConfigParams::m_language = selection.c_str();
        user_config->saveConfig();

        OptionsScreenUI::getInstance()->push();
    }
    else if(name == "music")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

        UserConfigParams::m_music = w->getState();
        Log::info("OptionsScreenAudio", "Music is now %s", ((bool) UserConfigParams::m_music) ? "on" : "off");

        if(w->getState() == false)
            music_manager->stopMusic();
        else
            music_manager->startMusic();
    }
    else if(name == "sound_effects")
    {
        CheckBoxWidget* w = dynamic_cast<CheckBoxWidget*>(widget);

        UserConfigParams::m_sfx = w->getState();
        SFXManager::get()->toggleSound(UserConfigParams::m_sfx);

        if (UserConfigParams::m_sfx)
        {
            SFXManager::get()->quickSound("horn");
        }
    }
    else if (name == "touch_device")
    {
        CheckBoxWidget* buttons_en = getWidget<CheckBoxWidget>("touch_device");
        assert(buttons_en != NULL);
        UserConfigParams::m_multitouch_mode = buttons_en->getState() ? 1 : 0;
    }

}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenUI::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenUI::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------

