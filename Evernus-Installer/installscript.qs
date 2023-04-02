/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the FOO module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

function Component()
{
    // Related to custom License agreement ui checkboxes
    gui.pageWidgetByObjectName("LicenseAgreementPage").entered.connect(changeLicenseLabels);

    // Related to dynamic installer page example
    // Page in inserted just before target directory selection
    component.loaded.connect(this, Component.prototype.loaded);
    if (!installer.addWizardPage(component, "Page", QInstaller.TargetDirectory))
        console.log("Could not add the dynamic page.");
}

// Related to custom License agreement ui checkboxes
changeLicenseLabels = function()
{
    page = gui.pageWidgetByObjectName("LicenseAgreementPage");
    page.AcceptLicenseLabel.setText("Yes I do! - Custom accept");
    page.RejectLicenseLabel.setText("No I don't! - Custom reject");
}

// From Here - Related to dynamic installer page example

Component.prototype.createOperations = function()
{
    try {
        // call the base create operations function
        component.createOperations();
        if (systemInfo.productType === "windows") {
            component.addOperation("CreateShortcut", "@TargetDir@/bin/evernus.exe", "@DesktopDir@/Evernus.lnk");
            component.addOperation("CreateShortcut", "@TargetDir@/bin/evernus.exe", "@StartMenuDir@/Evernus/Evernus.lnk");
    }
    } catch (e) {
        console.log(e);
    }
}

Component.prototype.loaded = function ()
{
    var page = gui.pageByObjectName("DynamicPage");
    if (page != null) {
        console.log("Connecting the dynamic page entered signal.");
        page.entered.connect(Component.prototype.dynamicPageEntered);
    }
}

Component.prototype.dynamicPageEntered = function ()
{
    var pageWidget = gui.pageWidgetByObjectName("DynamicPage");
    if (pageWidget != null) {
        console.log("Setting the widgets label text.")
        pageWidget.m_pageLabel.text = "This is a label in a dynamically created page.";
    }
}


