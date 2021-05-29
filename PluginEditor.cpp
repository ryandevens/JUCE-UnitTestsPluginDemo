/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
UnitTestPluginAudioProcessorEditor::UnitTestPluginAudioProcessorEditor (UnitTestPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setOpaque(true);

    addAndMakeVisible(startTestButton);
    startTestButton.onClick = [this] { start(); };

    addAndMakeVisible(testResultsBox);
    testResultsBox.setMultiLine(true);
    testResultsBox.setFont(Font(Font::getDefaultMonospacedFontName(), 12.0f, Font::plain));

    addAndMakeVisible(categoriesBox);
    categoriesBox.addItem("All Tests", 1);

    auto categories = UnitTest::getAllCategories();
    categories.sort(true);
   
    categoriesBox.addItemList(categories, 2);
    categoriesBox.setSelectedId(1);

    logMessage("This panel runs the built-in JUCE unit-tests from the selected category.\n");
    logMessage("To add your own unit-tests, see the JUCE_UNIT_TESTS macro.");

    setSize(500, 500);
}

UnitTestPluginAudioProcessorEditor::~UnitTestPluginAudioProcessorEditor()
{
    stopTest();
}

//==============================================================================
void UnitTestPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::grey);
}

void UnitTestPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(6);

    auto topSlice = bounds.removeFromTop(25);
    startTestButton.setBounds(topSlice.removeFromLeft(200));
    topSlice.removeFromLeft(10);
    categoriesBox.setBounds(topSlice.removeFromLeft(250));

    bounds.removeFromTop(5);
    testResultsBox.setBounds(bounds);
}

void UnitTestPluginAudioProcessorEditor::start()
{
    startTest(categoriesBox.getText());
}

void UnitTestPluginAudioProcessorEditor::startTest(const String& category)
{
    testResultsBox.clear();
    startTestButton.setEnabled(false);

    currentTestThread.reset(new TestRunnerThread(*this, category));
    currentTestThread->startThread();
}

void UnitTestPluginAudioProcessorEditor::stopTest()
{
    if (currentTestThread.get() != nullptr)
    {
        currentTestThread->stopThread(15000);
        currentTestThread.reset();
    }
}

void UnitTestPluginAudioProcessorEditor::logMessage(const String& message)
{
    testResultsBox.moveCaretToEnd();
    testResultsBox.insertTextAtCaret(message + newLine);
    testResultsBox.moveCaretToEnd();
}

void UnitTestPluginAudioProcessorEditor::testFinished()
{
    stopTest();
    startTestButton.setEnabled(true);
    logMessage(newLine + "*** Tests finished ***");
}