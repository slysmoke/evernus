function getSourceName(value) {
    switch (value) {
    case 0:
        return qsTr("Buy from source");
    case 1:
        return qsTr("Manufacture");
    case 2:
        return qsTr("Acquire for free");
    case 3:
        return qsTr("Buy at custom cost");
    case 4:
        return qsTr("Take assets then buy from source");
    case 5:
        return qsTr("Take assets then manufacture");
    case 6:
        return qsTr("Take assets then buy at custom cost");
    }

    return "";
}

function formatDuration(duration) {
    var hours   = Math.floor(duration / 3600);
    var minutes = Math.floor((duration / 60) % 60);
    var seconds = duration % 60;

    if (hours < 10)
        hours   = "0" + hours;
    if (minutes < 10)
        minutes = "0" + minutes;
    if (seconds < 10)
        seconds = "0" + seconds;

    return "%1:%2:%3".arg(hours).arg(minutes).arg(seconds);
}

function formatCurrency(value) {
    return value.toLocaleCurrencyString(Qt.locale(), (omitCurrencySymbol) ? (" ") : (qsTr("ISK")));
}
