/*
==============================================================================

PopupMenuSearch.cpp
Created: 2 Oct 2021 6:07:38pm
Author:  regalmoix

==============================================================================
*/

#include "PopupMenuSearch.h"

/* --------------------- Function --------------------- */

void showPopupMenuWithQuickSearch(
                                    const PopupMenu&                    menu,
                                    const PopupMenuQuickSearchOptions&  options,
                                    Component*                          targetComponent,
                                    std::function<void (int)>           userCallback
                                )
{
    new PopupMenuQuickSearch (menu, options, targetComponent, userCallback);
}

/* --------------------- PopupMenuQuickSearchOptions --------------------- */

PopupMenuQuickSearchOptions::PopupMenuQuickSearchOptions () 
{

}

PopupMenuQuickSearchOptions::PopupMenuQuickSearchOptions (PopupMenu::Options opt) 
    : PopupMenu::Options (opt) 
{

}

/* --------------------- MenuItemComponent --------------------- */

PopupMenuQuickSearch::QuickSearchComponent::MenuItemComponent::MenuItemComponent (PopupMenuQuickSearch* owner) 
                : owner (owner) 
{

}

void PopupMenuQuickSearch::QuickSearchComponent::MenuItemComponent::paint (Graphics& g) 
{
    getLookAndFeel()
        .drawPopupMenuItem (
            g, getLocalBounds(), 
            false, 
            searchItem.popupMenuItem->isEnabled, 
            highlighted, 
            searchItem.popupMenuItem->isTicked, 
            false, 
            searchItem.label,
            searchItem.popupMenuItem->shortcutKeyDescription, 
            searchItem.popupMenuItem->image.get(), 
            searchItem.popupMenuItem->colour.isTransparent() ? nullptr : &searchItem.popupMenuItem->colour
        );
}

void PopupMenuQuickSearch::QuickSearchComponent::MenuItemComponent::updateWith (QuickSearchItem& new_e, bool new_highlighted)
{
    if (new_e.popupMenuItem != searchItem.popupMenuItem || highlighted != new_highlighted)
    {
        this->searchItem             = new_e;
        this->highlighted   = new_highlighted;
        repaint();
    }
}

void PopupMenuQuickSearch::QuickSearchComponent::MenuItemComponent::mouseUp (const MouseEvent& event) 
{
    if (! event.mouseWasDraggedSinceMouseDown())
    {
        if (searchItem.popupMenuItem->isEnabled)
            owner->quickSearchFinished (searchItem.id);
    }
}
            
void PopupMenuQuickSearch::QuickSearchComponent::MenuItemComponent::mouseExit (const MouseEvent&) 
{
    highlighted = false;
    repaint();
}

void PopupMenuQuickSearch::QuickSearchComponent::MenuItemComponent::mouseEnter (const MouseEvent&) 
{
    highlighted = true;
    repaint();
}

/* --------------------- QuickSearchComponent --------------------- */

PopupMenuQuickSearch::QuickSearchComponent::QuickSearchComponent (PopupMenuQuickSearch* owner, String& initialString) 
    : owner (owner)
{
    auto* target            = owner->targetComponentWeakReference.get();
    auto  targetScreenArea  = owner->options.getTargetScreenArea();

    jassert     (target);
    setOpaque   (true);

    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);
    setAlwaysOnTop (true);

    creationTime            = Time::getCurrentTime();

    readPopupMenuItems      (menuTree, owner->menu);
    handleDuplicatedLabels  ();

    /* compute the width and item height */
    String longestString;
    for (auto& q : quickSearchItems)
    {
        if (q.label.length() > longestString.length())
            longestString = q.label;
    }
    getLookAndFeel().getIdealPopupMenuItemSize (longestString, false, owner->options.getStandardItemHeight(), itemWidth, itemHeight);

    // it is always nice to have a windows aligned with the target button
    if (itemWidth < targetScreenArea.getWidth() && targetScreenArea.getWidth() < 300)
        itemWidth = targetScreenArea.getWidth();

    // the actual height will be adjusted later
    setBounds (targetScreenArea.getX(), targetScreenArea.getY(), itemWidth, itemHeight);

    Font font = getLookAndFeel().getPopupMenuFont();

    // this is what does LookAndFeel::drawPopupMenuItem
    if (font.getHeight() > (itemHeight - 2) / 1.3f)
        font.setHeight ((itemHeight - 2) / 1.3f);

    textColour = getLookAndFeel().findColour (PopupMenu::textColourId);

    searchLabel.setText   (TRANS ("Search:"), dontSendNotification);
    searchLabel.setColour (Label::textColourId, textColour.withAlpha (0.5f));
    searchLabel.setFont   (font);

    searchLabel.setJustificationType (Justification::bottomLeft);
    searchLabel.setSize (searchLabel.getBorderSize().getLeftAndRight() + int (font.getStringWidth (searchLabel.getText())), itemHeight);

    addAndMakeVisible (searchLabel);

    editor.setBounds   (searchLabel.getRight(), 0, itemWidth - searchLabel.getRight(), itemHeight);
    editor.setFont     (font);
    editor.addListener (this);

    editor.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::textColourId, textColour);
    editor.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::focusedOutlineColourId, Colours::transparentBlack);
    editor.setColour (CaretComponent::caretColourId, textColour);

    addAndMakeVisible(editor);

    editor.setText(initialString, dontSendNotification);

    editor.addKeyListener(this);

    startTimer(100); // will grab the keyboard focus for the texteditor.

    updateContent();
}

void PopupMenuQuickSearch::QuickSearchComponent::readPopupMenuItems (MenuTree& tree, const PopupMenu& menu)
{
    PopupMenu::MenuItemIterator it (menu);

    while (it.next())
    {
        auto& item  = it.getItem();

        if (item.subMenu)
        {
            MenuTree child;
            child.label     = item.text;
            child.parent    = &tree;

            tree.childs.push_back (child);
            readPopupMenuItems (tree.childs.back(), *item.subMenu);
        }
        else if (item.itemID > 0)
        {
            QuickSearchItem q {item.itemID, item.text, &item, &tree};

            auto iter = owner->options.itemsToIgnoreOrRenameInQuickSearch.find (q.id);

            // the label is renamed, or just set to empty string if we want to ignore this entry.
            if (iter != owner->options.itemsToIgnoreOrRenameInQuickSearch.end())
                q.label = iter->second;

            // ignore entries with no label
            if (q.label.isEmpty())
                continue;

            quickSearchItems.push_back (q);
        }
    }
}

void PopupMenuQuickSearch::QuickSearchComponent::handleDuplicatedLabels()
{
    if (!owner->options.mergeEntriesWithSameLabel)
    {
        // use name of parent menu to disambiguate duplicates
        vector<MenuTree*> parents (quickSearchItems.size());

        for (size_t idx = 0; idx < quickSearchItems.size(); ++idx)
            parents[idx] = quickSearchItems[idx].menu;

        while (true)
        {
            bool done = true;
            std::map<int64, std::list<size_t>> hashes;

            for (size_t idx = 0; idx < quickSearchItems.size(); ++idx)
            {
                auto& q = quickSearchItems[idx];
                auto  h = q.label.hashCode64();

                hashes[h].push_back(idx);

                if (hashes[h].size() > 1 && parents[idx] != nullptr)
                    done = false;      
            }

            if (done)
                break;

            else
            {
                bool changed_something = false;
                for (auto hh : hashes)
                {
                    if (hh.second.size() > 1)
                    {
                        for (auto idx : hh.second)
                        {
                            auto& q = quickSearchItems.at (idx);
                            if (parents[idx] && parents[idx]->label.isNotEmpty())
                            {
                                q.label = parents[idx]->label + " / " + q.label;
                                parents[idx] = parents[idx]->parent;
                                changed_something = true;
                            }
                        }
                    }
                }
                if (!changed_something)
                    break;
            }
        }
    }
    else
    {
        // remove all duplicates
        std::set<int64>         hashes;
        vector<QuickSearchItem> v;

        for (auto& q : quickSearchItems)
        {
            auto h = q.label.hashCode64();
            if (hashes.count (h) == 0)
            {
                hashes.insert (h);
                v.push_back (q);
            }
        }
        quickSearchItems.swap (v);
    }
}

Rectangle<int> PopupMenuQuickSearch::QuickSearchComponent::getBestBounds (int totalH)
{
    auto tr         = owner->options.getTargetScreenArea();

    auto screenArea = Desktop::getInstance().getDisplays().findDisplayForPoint (tr.getCentre()).userArea;

    auto spaceUnder = screenArea.getBottom() - tr.getBottom();
    auto spaceOver  = tr.getY() - screenArea.getY();

    if (spaceUnder >= 0.8 * spaceOver)
        displayedOverOrUnder = 1;

    else
        displayedOverOrUnder = -1;

    if (displayedOverOrUnder == -1)
    {
        totalH = std::min (totalH, spaceOver);
        return { tr.getX(), tr.getY() - totalH, getWidth(), totalH };
    }

    else
    {
        totalH = std::min (totalH, spaceUnder);
        return { tr.getX(), tr.getBottom(), getWidth(), totalH };
    }
}

void PopupMenuQuickSearch::QuickSearchComponent::updateContent()
{
    updateMatches();

    int nbVisibleMatches = std::min (owner->options.maxNumberOfMatchesDisplayed, (int) matches.size());

    int h = itemHeight;
    jassert (h);

    int separator_height = itemHeight / 2;

    int max_total_h = h + std::max (1, nbVisibleMatches) * h + separator_height;

    auto rect = getBestBounds (max_total_h);

    nbVisibleMatches = std::min (
                                    nbVisibleMatches,
                                    std::max (1, (rect.getHeight() - h - separator_height) / h)
                                );

    int total_h = h + separator_height + std::max (1, nbVisibleMatches) * h;
    setBounds (getBestBounds (total_h));
    total_h = getHeight();

    if (displayedOverOrUnder == 1)
    {
        searchLabel.setTopLeftPosition (0, 0);
        editor.setBounds (editor.getX(), 0, editor.getWidth(), h);
    }

    else
    {
        searchLabel.setTopLeftPosition (0, total_h - h);
        editor.setBounds (editor.getX(), total_h - h, editor.getWidth(), h);
    }

    bestItems.resize (nbVisibleMatches);
    for (int i = 0; i < nbVisibleMatches; ++i)
    {
        if (bestItems.at (i) == nullptr)
        {
            bestItems[i] = std::make_unique<MenuItemComponent> (owner);
            addAndMakeVisible (*bestItems[i]);
        }

        int ii = firstDisplayedMatch + i;
        bestItems[i]->updateWith (quickSearchItems.at (matches.at (ii)), ii == highlightedMatch);

        if (displayedOverOrUnder == 1)
            bestItems[i]->setBounds (0, (h + separator_height) + i * h, itemWidth, h);

        else
            bestItems[i]->setBounds (0, total_h - (h + separator_height) - (i + 1) * h, itemWidth, h);
    }
}

int PopupMenuQuickSearch::QuickSearchComponent::evalMatchScore (String str, const String& needle)
{
    if (needle.isEmpty())
        return 0;

    int score       = 0;
    int oldBestJ    = -1;

    for (int i = 0; i < needle.length();)
    {
        int bestJ   = -1;
        int bestLen =  0;

        for (int j = 0; j < str.length(); ++j)
        {
            int len = 0;
            while (i + len < needle.length() && j + len < str.length() && CharacterFunctions::compareIgnoreCase (str[j + len], needle[i + len]) == 0)
                ++len;
            
            if (len > bestLen)
            {
                bestLen = len;
                bestJ = j;
            }
        }

        // char not found ! no need to continue
        if (bestLen == 0)
            return NO_MATCH_SCORE; 
        
        // boost for matches of more that one char
        score += (bestLen == 1) ? 1 : (bestLen * bestLen + 1); 
        
        if ((bestLen == 1 && str[bestJ] != ' ') || bestJ < oldBestJ)
            score -= 100;

        // mark these characters are 'done' , so that 'xxxx' does not match 'x'
        str = str.replaceSection (bestJ, bestLen, "\t"); 
        
        oldBestJ = bestJ;

        i += std::max (1, bestLen);
    }
    return score;
}

void PopupMenuQuickSearch::QuickSearchComponent::updateMatches()
{
    String needle       = editor.getText();
    auto   oldMatches   = matches;
    matches.resize (0);

    vector<int> scores(quickSearchItems.size(), 0);

    for (size_t idx = 0; idx < quickSearchItems.size(); ++idx)
    {
        auto& q = quickSearchItems[idx];

        scores[idx] = evalMatchScore (q.label, needle);

        if (!q.popupMenuItem->isEnabled)
            scores[idx] -= 10000;

        matches.push_back (idx);
    }

    auto matchesComparator =    [&scores] (size_t a, size_t b) 
                                { 
                                    return scores[a] > scores[b]; 
                                };

    std::stable_sort (matches.begin(), matches.end(), matchesComparator);

    int threshold = !matches.empty() && scores[matches.front()] > 0 ? 0 : NO_MATCH_SCORE;

    while (! matches.empty() && scores[matches.back()] <= threshold)
        matches.pop_back();

    if (matches != oldMatches)    
        firstDisplayedMatch = highlightedMatch = 0;
}

void PopupMenuQuickSearch::QuickSearchComponent::paint (Graphics& g) 
{
    getLookAndFeel().drawPopupMenuBackground (g, getWidth(), getHeight());
    g.setColour (textColour.withAlpha (0.4f));

    int ySeparator = itemHeight + itemHeight / 4;

    if (displayedOverOrUnder == -1)
        ySeparator = getHeight() - ySeparator;

    g.drawHorizontalLine (ySeparator, itemHeight / 2, getWidth() - itemHeight / 2);
    
    if (matches.empty())
    {
        g.setFont   (searchLabel.getFont());
        g.setColour (textColour.withAlpha (0.5f));

        int y0 = (displayedOverOrUnder == 1 ? (getHeight() - itemHeight) : 0);

        g.drawText (TRANS ("(no match)"), Rectangle<int> (0, y0, itemWidth, itemHeight), Justification::centred);
    }
}

void PopupMenuQuickSearch::QuickSearchComponent::textEditorReturnKeyPressed (TextEditor&) 
{
    if (!matches.empty())
    {
        auto& q = quickSearchItems.at (matches.at (highlightedMatch));
        if (q.popupMenuItem->isEnabled)
            owner->quickSearchFinished (q.id);
    }
}

void PopupMenuQuickSearch::QuickSearchComponent::textEditorEscapeKeyPressed (TextEditor&)
{ 
    owner->quickSearchFinished(0);
}

void PopupMenuQuickSearch::QuickSearchComponent::textEditorTextChanged (TextEditor&)  
{
    updateContent();
}

bool PopupMenuQuickSearch::QuickSearchComponent::keyPressed (const KeyPress& key)
{ 
    return Component::keyPressed (key);
}

bool PopupMenuQuickSearch::QuickSearchComponent::keyPressed (const KeyPress& key, Component*) 
{
    if (key == '\t')
        owner->showPopupMenu();

    bool up     = (key == KeyPress::upKey);
    bool down   = (key == KeyPress::downKey);

    if (displayedOverOrUnder == -1)
        std::swap (up, down);

    if (up)
    {
        while (highlightedMatch > 0)
        {
            --highlightedMatch;

            if (firstDisplayedMatch > highlightedMatch)
                firstDisplayedMatch = highlightedMatch;
            
            updateContent();
        }
        return true;
    }

    if (down)
    {
        if (highlightedMatch + 1 < (int) matches.size())
        {
            ++highlightedMatch;

            if (highlightedMatch - firstDisplayedMatch >= (int) bestItems.size())
            {
                firstDisplayedMatch = highlightedMatch - (int) bestItems.size() + 1;
                jassert (firstDisplayedMatch >= 0);
            }

            auto& q = quickSearchItems.at (matches.at (highlightedMatch));

            if (! q.popupMenuItem->isEnabled)
                highlightedMatch = 0;
            updateContent();
        }
        return true;
    }
    return false;
}

void PopupMenuQuickSearch::QuickSearchComponent::timerCallback() 
{
    editor.grabKeyboardFocus();
}

void PopupMenuQuickSearch::QuickSearchComponent::inputAttemptWhenModal() 
{
    /**
     * @note
     * I get sometimes spurious calls to inputAttemptWhenModal when switching from PopupMenu to
     * QuickSearchComponent, just after the creation of QuickSearchComponent, so that's why there
     * is a delay here.
     */
    double dt = (Time::getCurrentTime() - creationTime).inSeconds();
    if (dt > 0.2)
        owner->quickSearchFinished (0);
}

/* --------------------- PopupMenuQuickSearch --------------------- */

PopupMenuQuickSearch::PopupMenuQuickSearch (const PopupMenu& menu_, const PopupMenuQuickSearchOptions& options_, Component* targetComponent, std::function<void (int)> user_callback_)
{
    menu                        = menu_;
    options                     = options_;
    userCallback                = user_callback_;
    targetComponentWeakReference= targetComponent;

    // for managing the lifetime of PopupMenuQuickSearch we require that a targetComponent is provided
    jassert (targetComponentWeakReference.get());

    targetComponentWeakReference->addComponentListener (this);

    if (!options.startInQuickSearchMode)
        showPopupMenu();
    else
        showQuickSearch();
}

PopupMenuQuickSearch::~PopupMenuQuickSearch()
{
    if (targetComponentWeakReference)            
        targetComponentWeakReference->removeComponentListener (this);

    for (auto c : listenedComponents)
    {
        if (c.get())
            c->removeKeyListener (this);                
    }
}

void PopupMenuQuickSearch::showPopupMenu()
{
    if (quickSearch)
    {
        quickSearch            = 0;
        keyPressedWhileMenu  = "";
    }
    menu.showMenuAsync  (options,   [this] (int result) 
                                    { 
                                        popupMenuFinished (result);
                                    }
                        );
    startTimer (20); // follow the current modal component and intercepts keyPressed events
}

void PopupMenuQuickSearch::showQuickSearch()
{
    if (quickSearch == nullptr && targetComponentWeakReference)
    {
        quickSearch = std::make_unique<QuickSearchComponent> (this, keyPressedWhileMenu);
        
        // userCallback won't run, since quickSearch != 0
        PopupMenu::dismissAllActiveMenus();

        quickSearch->setAlwaysOnTop    (true);
        quickSearch->setVisible        (true);
        quickSearch->addToDesktop      (0);
        quickSearch->enterModalState   (true);
    }
}

void PopupMenuQuickSearch::popupMenuFinished (int result)
{
    if (!quickSearch)
    {
        if (targetComponentWeakReference.get())
            userCallback (result);
        delete this;
    }
}

void PopupMenuQuickSearch::quickSearchFinished (int result)
{
    if (quickSearch)
    {
        quickSearch = 0;
        if (targetComponentWeakReference.get())
        {
            userCallback (result);
        }
        // delete only when quick_search is active, otherwise this is deleted by the popupmenu callback
        delete this; 
    }
}

void PopupMenuQuickSearch::componentBeingDeleted (Component&)  
{ 
    quickSearchFinished (true); 
}

bool PopupMenuQuickSearch::keyPressed (const KeyPress& key, Component* originatingComponent) 
{
    auto c = key.getTextCharacter();
    if (c > ' ' || c == '\t')
    {
        if (c != '\t')
            keyPressedWhileMenu += c;
        
        showQuickSearch();
        
        if (quickSearch)
            return true;
    }
    return false;
}

void PopupMenuQuickSearch::timerCallback() 
{
    if (!quickSearch)
    {
        auto c = Component::getCurrentlyModalComponent();

        if (! c)
            return;

        // check if already listened to, and remove deleted components
        for (auto it = listenedComponents.begin(); it != listenedComponents.end();)
        {
            auto itn = it;
            ++itn;

            if (it->get() == nullptr)
                listenedComponents.erase (it);

            if (it->get() == c)
                return;                     // this comp is alreadly listened to

            it = itn;
        }

        listenedComponents.push_back (c);
        c->addKeyListener (this);
    }
}

