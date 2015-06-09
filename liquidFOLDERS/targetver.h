// In diesem MFC-Beispielquellcode wird die Verwendung der MFC Microsoft Office Fluent-Benutzeroberfläche 
// ("Fluent-Benutzeroberfläche") demonstriert. Der Beispielquellcode wird ausschließlich zu Referenzzwecken und als Ergänzung zur 
// Microsoft Foundation Classes-Referenz und zugehöriger elektronischer Dokumentation 
// bereitgestellt, die in der MFC C++-Bibliotheksoftware enthalten sind.  
// Lizenzbedingungen zum Kopieren, Verwenden oder Verteilen der Fluent-Benutzeroberfläche sind separat erhältlich. 
// Weitere Informationen zum Lizenzierungsprogramm für die Fluent-Benutzeroberfläche finden Sie unter 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// Alle Rechte vorbehalten.

#pragma once

// Die folgenden Makros definieren die mindestens erforderliche Plattform. Die mindestens erforderliche Plattform
// ist die früheste Windows-, Internet Explorer-Version usw., die über die erforderlichen Features zur Ausführung 
// Ihrer Anwendung verfügt. Die Makros aktivieren alle Features, die auf den Plattformversionen bis 
// einschließlich der angegebenen Version verfügbar sind.

// Ändern Sie folgende Definitionen für Plattformen, die älter als die unten angegebenen sind.
// Unter MSDN finden Sie die neuesten Informationen über die entsprechenden Werte für die unterschiedlichen Plattformen.
#ifndef WINVER                          // Gibt an, dass Windows Vista die mindestens erforderliche Plattform ist.
#define WINVER 0x0600           // Ändern Sie den entsprechenden Wert, um auf andere Versionen von Windows abzuzielen.
#endif

#ifndef _WIN32_WINNT            // Gibt an, dass Windows Vista die mindestens erforderliche Plattform ist.
#define _WIN32_WINNT 0x0600     // Ändern Sie den entsprechenden Wert, um auf andere Versionen von Windows abzuzielen.
#endif

#ifndef _WIN32_WINDOWS          // Gibt an, dass Windows 98 die mindestens erforderliche Plattform ist.
#define _WIN32_WINDOWS 0x0410 // Ändern Sie den entsprechenden Wert, um auf mindestens Windows Me abzuzielen.
#endif

#ifndef _WIN32_IE                       // Gibt an, dass Internet Explorer 7.0 die mindestens erforderliche Plattform ist.
#define _WIN32_IE 0x0700        // Ändern Sie den entsprechenden Wert, um auf andere Versionen von IE abzuzielen.
#endif
