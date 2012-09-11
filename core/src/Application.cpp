/* Copyright (c) 2012 Gillis Van Ginderachter <supergillis@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Application.h"
#include "Hook.h"

#include <Position.h>

#include <QMessageBox>

int Application::argc_ = 0;

Application::Application():
    QApplication(Application::argc_, NULL) {
    QApplication::setApplicationName("Tibia Hook");
    QApplication::setApplicationVersion("beta");

    qRegisterMetaType<Position>("Position");

    setQuitOnLastWindowClosed(false);

    try {
        // Construct new application first, and then construct the hook
        hook_ = new Hook();
    }
    catch(std::exception& exception) {
        QMessageBox message;
        message.setWindowTitle(QApplication::applicationName());
        message.setText("Something terrible has happened!");
        message.setDetailedText(exception.what());
        message.setDefaultButton(QMessageBox::Ignore);
        message.exec();
    }
}

Application::~Application() {
    delete hook_;
    hook_ = NULL;
}