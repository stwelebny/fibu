function getCookie(name) {
    const value = "; " + document.cookie;
    const parts = value.split("; " + name + "=");
    if (parts.length === 2) return parts.pop().split(";").shift();
}

document.addEventListener('DOMContentLoaded', function() {


    const client = getCookie('mandant');
    if (!client) {
        alert('Bitte einen Mandanten angeben!');
        return;
    }
    fetchJournalData();
});




function formatCurrencyValue(value) {
    return new Intl.NumberFormat('de-AT', {
        minimumFractionDigits: 2,
        maximumFractionDigits: 2
    }).format(value);
}


function displayJournalData(data) {
    const tableBody = document.getElementById('journalDetails');
    tableBody.innerHTML = ''; // Clear any previous data

    data.forEach(entry => {
        const tableRow = document.createElement('tr');

        tableRow.innerHTML = `
            <td>${entry.Belegdatum}</td>
            <td>${entry.Belegnummer}</td>
            <td>${entry.SollKonto}</td>
            <td>${entry.HabenKonto}</td>
            <td>${entry.Text}</td>
            <td style="text-align: right;">${formatCurrencyValue(entry["Betrag"])}</td>
        `;

        tableBody.appendChild(tableRow);
    });
}


function fetchJournalData() {
    const client = getCookie('mandant');

    if (!client) {
        alert('Bitte einen Mandanten angeben!');
        return;
    }

    // Make an AJAX call to the CGI script
    fetch(`./cgi-bin/journalReport?client=${client}`)
        .then(response => response.json())
        .then(data => {
            displayJournalData(data);
        })
        .catch(error => {
            console.error('Fehler bei der Journalabfrage', error);
        });
}


