
#pragma once

#include <QWidget>
#include <uiThread.h>

// FULLY STATIC CLASS
class VMessageBox
{
  public:
    static uint16_t information(QWidget* parent, const char* title, const char* message, uint16_t flags,
                                      uint16_t defaultAnswer);
    static uint16_t question(QWidget* parent, const char* title, const char* message, uint16_t flags,
                                   uint16_t defaultAnswer);
    static uint16_t warning(QWidget* parent, const char* title, const char* message, uint16_t flags,
                                  uint16_t defaultAnswer);
    static uint16_t critical(QWidget* parent, const char* title, const char* message, uint16_t flags,
                                   uint16_t defaultAnswer);
    static uint16_t informationSystemModal(QWidget* parent, const char* title, const char* message,
                                                 uint16_t flags, uint16_t defaultAnswer);

  private:
    static uint16_t _displayBox(QWidget* parent, const char* title, const char* message, uint16_t flags);
};
