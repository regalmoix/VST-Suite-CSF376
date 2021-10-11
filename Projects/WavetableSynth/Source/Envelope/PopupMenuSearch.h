/*
    ==============================================================================

    PopupMenuSearch.h
    Created: 2 Oct 2021 8:08:17pm
    Author:  Juce Forum : Link below

    ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include <set>
#include <map>
#include <vector>

using std::vector;
using std::unique_ptr;
using std::map;

/**
 * @brief Refactored copy of the code lifted from Juce Forum with Minor Tweaks
 * @link  https://forum.juce.com/t/augmenting-popupmenu-with-a-quick-search-option/42177
 */

/** @details
    An almost drop-in replacement for PopupMenu::showMenuAsync, that adds a "quick search" interface
    to the PopupMenu: when the PopupMenu is shown, any character typed on the keyboard will switch to
    the "quick search" view and start filtering the PopupMenu entries. Sub-menus are handled
    (everything is flattened into a giant list). The <tab> key switches back and forth between
    PopupMenu and QuickSearch view.

    The search is case-insensitive, and looks for multiple partial matches: for example "nelan"
    matches "Ned Flanders".

    Mouse and arrow keys can also be used while in quicksearch.

    Options are provided by the PopupMenuQuickSearchOptions, which is a subclass of
    PopupMenu::Options.

    If some entries are not relevant when in quick-search mode, or if some of them must be relabeled,
    the PopupMenuQuickSearchOptions::itemsToIgnoreOrRenameInQuickSearch option provides a way to
    achieve that.

    If some popup menu items are duplicated, an option is also available to merge them. Otherwise, it
    will try to disambiguate them by adding (if possible) the name of the parent sub-menu to the
    label.

    Limitations:

    - a target component has to be provided (via the PopupMenu::Options). It is used to avoid leaking
    memory (the PopupMenuQuickSearch structure is automatically destroyed if the targetComponent is
    deleted). A better solution would be to detect when PopupMenu::dismissAllActiveMenus is called
    but that is not possible.

    If you need to call PopupMenu::Options::withTargetScreenArea, call it *after*
    PopupMenu::Options::withTargetComponent (that way getTargetComponent return non-null, but
    getTargetScreenArea is the correct one).

    - the QuickSearchComponent is a desktop window, it does not have a juce parent (should be
    relativily easy to fix it to handle that case when PopupMenu::Options.getParentComponent is
    non-null).

    Hacks:

    - The way key press are intercepted from PopupMenu is not very elegant (registering a KeyListener
    to the currentlyModalWindow in a timer callback). But it seems to work very well.

    - Giving focus to the texteditor of QuickSearchComponent (grabKeyboardFocus in a timerCallback). I
    always have trouble with JUCE for this kind of stuff


    License: CC0 ( https://creativecommons.org/publicdomain/zero/1.0/ )
*/

struct PopupMenuQuickSearchOptions: public PopupMenu::Options
{
    bool startInQuickSearchMode     = false;
    bool mergeEntriesWithSameLabel  = false;
    int maxNumberOfMatchesDisplayed = 60;

    map<int, String>    itemsToIgnoreOrRenameInQuickSearch;

    PopupMenuQuickSearchOptions ();

    PopupMenuQuickSearchOptions (PopupMenu::Options opt);
};

void showPopupMenuWithQuickSearch(
                                    const PopupMenu&                    menu,
                                    const PopupMenuQuickSearchOptions&  options,
                                    Component*                          targetComponent,
                                    std::function<void (int)>           userCallback
                                );
/* --------------------- implementation --------------------- */

/** @brief This struct is automatically destroyed when the PopupMenu or QuickSearchComponent completes, or when the target component is destroyed */
struct PopupMenuQuickSearch  :  public KeyListener, 
                                public Timer, 
                                public ComponentListener
{
    class QuickSearchComponent: public Component,
                                public Timer,
                                public TextEditor::Listener,
                                public KeyListener
    {  
        /** @brief hierarchy of submenus in the PopupMenu (used when disambiguating duplicated label) */
        struct MenuTree
        {
            String              label;
            MenuTree*           parent          { nullptr };
            std::list<MenuTree> childs;
        };

        struct QuickSearchItem
        {
            int                 id;
            String              label;
            PopupMenu::Item*    popupMenuItem { nullptr };
            MenuTree*           menu;
        };

        struct MenuItemComponent : public Component
        {
            QuickSearchItem         searchItem;
            bool                    highlighted     { false };
            PopupMenuQuickSearch*   owner;

            MenuItemComponent   (PopupMenuQuickSearch* owner);

            void paint          (Graphics& g) override;

            void updateWith     (QuickSearchItem& newSearchItem, bool newHighlighted);

            void mouseUp        (const MouseEvent& event) override;
            
            void mouseExit      (const MouseEvent&) override;
            
            void mouseEnter     (const MouseEvent&) override;
        };

        MenuTree                menuTree;
        Label                   searchLabel;
        TextEditor              editor;
        PopupMenuQuickSearch*   owner;

        vector<QuickSearchItem> quickSearchItems;
        vector<size_t>          matches;

        int                     firstDisplayedMatch     = 0; 
        int                     highlightedMatch        = 0;

        int                     itemWidth               = 0; 
        int                     itemHeight              = 0;

        // -1 over, 1 under, 0 undecided
        int                     displayedOverOrUnder = 0; 

        Colour                  textColour;
        Time                    creationTime;

        vector<unique_ptr<MenuItemComponent>> bestItems;

    public:
        const int               NO_MATCH_SCORE          = -1000000;

        QuickSearchComponent (PopupMenuQuickSearch* owner, String& initialString);

        /** @brief Recursively parse the PopupMenu items */
        void readPopupMenuItems (MenuTree& tree, const PopupMenu& menu);

        void handleDuplicatedLabels();

        /** @brief decide the orientation and dimensions of the QuickSearchComponent */
        Rectangle<int> getBestBounds (int totalH);

        void updateContent();

        /** @brief give a score for the match between searched string 'needle' and 'str' */
        int evalMatchScore (String str, const String& needle);

        /** @brief get the the list of best matches, sorted by decreasing score */
        void updateMatches();

        void paint (Graphics& g) override;

        void textEditorReturnKeyPressed (TextEditor&) override;

        void textEditorEscapeKeyPressed (TextEditor&) override;

        void textEditorTextChanged (TextEditor&) override;

        /** @brief just to silence a clang warning about the other overriden keyPressed member function */
        bool keyPressed (const KeyPress& key) override;

        /** @brief capture KeyPress events of the TextEditor */
        bool keyPressed (const KeyPress& key, Component*) override;

        /** @brief I always have some trouble to manage keyboard focus with JUCE. */
        void timerCallback() override;

        void inputAttemptWhenModal() override;
    };

    PopupMenu                               menu;
    PopupMenuQuickSearchOptions             options;

    unique_ptr<QuickSearchComponent>        quickSearch;
    WeakReference<Component>                targetComponentWeakReference;

    std::function<void (int)>               userCallback;
    String                                  keyPressedWhileMenu;

    // just for security, keep a list all the component that we have added key listeners
    std::list<WeakReference<Component>>     listenedComponents;

    void showPopupMenu();

    void showQuickSearch();

    /** @brief Called by PopupMenu::showMenuAsync it has completed */
    void popupMenuFinished (int result);

    /** @brief called by QuickSearchComponent when it has completed */
    void quickSearchFinished (int result);

    /** @brief avoid leaks as quick search does not have many ways to know when to close */
    void componentBeingDeleted (Component&) override;

    /** @brief hijack the KeyPress received by the PopupMenu */
    bool keyPressed (const KeyPress& key, Component* originatingComponent) override;

    void timerCallback  () override;

    PopupMenuQuickSearch(
                            const PopupMenu& menu_, 
                            const PopupMenuQuickSearchOptions& options_, 
                            Component* targetComponent_, 
                            std::function<void (int)> userCallback_
                        );

    ~PopupMenuQuickSearch();

    JUCE_LEAK_DETECTOR (PopupMenuQuickSearch)
};

