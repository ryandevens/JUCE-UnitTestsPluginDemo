/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class UnitTestPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    UnitTestPluginAudioProcessorEditor (UnitTestPluginAudioProcessor&);
    ~UnitTestPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void start();
    void startTest(const String& category);
    void stopTest();
    void logMessage(const String& message);
    void testFinished();

private:
    //==============================================================================
    class TestRunnerThread : public Thread,
        private Timer
    {
    public:
        TestRunnerThread(UnitTestPluginAudioProcessorEditor& utd, const String& ctg)
            : Thread("Unit Tests"),
            owner(utd),
            category(ctg)
        {}

        void run() override
        {
            CustomTestRunner runner(*this);

            if (category == "All Tests")
                runner.runAllTests();
            else
                runner.runTestsInCategory(category);

            startTimer(50); // when finished, start the timer which will
                             // wait for the thread to end, then tell our component.
        }

        void logMessage(const String& message)
        {
            WeakReference<UnitTestPluginAudioProcessorEditor> safeOwner(&owner);

            MessageManager::callAsync([=]
                {
                    if (auto* o = safeOwner.get())
                        o->logMessage(message);
                });
        }

        void timerCallback() override
        {
            if (!isThreadRunning())
                owner.testFinished(); // inform the demo page when done, so it can delete this thread.
        }

    private:
        //==============================================================================
        // This subclass of UnitTestRunner is used to redirect the test output to our
        // TextBox, and to interrupt the running tests when our thread is asked to stop..
        class CustomTestRunner : public UnitTestRunner
        {
        public:
            CustomTestRunner(TestRunnerThread& trt) : owner(trt) {}

            void logMessage(const String& message) override
            {
                owner.logMessage(message);
            }

            bool shouldAbortTests() override
            {
                return owner.threadShouldExit();
            }

        private:
            TestRunnerThread& owner;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomTestRunner)
        };

        UnitTestPluginAudioProcessorEditor& owner;
        const String category;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestRunnerThread)
    };

    UnitTestPluginAudioProcessor& audioProcessor;
    std::unique_ptr<TestRunnerThread> currentTestThread;

    TextButton startTestButton{ "Run Unit Tests..." };
    ComboBox categoriesBox;
    TextEditor testResultsBox;

    void lookAndFeelChanged() override
    {
        testResultsBox.applyFontToAllText(testResultsBox.getFont());
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE(UnitTestPluginAudioProcessorEditor)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitTestPluginAudioProcessorEditor)
};
