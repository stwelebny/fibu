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
    fetch(`./cgi-bin/balanceReport?client=${client}`)
    .then(response => {
        if (!response.ok) {
            throw new Error("Network response was not ok");
        }
        return response.json();
    })
    .then(data => {
        renderBalanceSheet(data);
    })
    .catch(error => {
        console.error('There was a problem fetching balance sheet data:', error);
    });
});

function renderBalanceSheetSection(section, container) {
    // Create a header for the section
    if (section.sollSum != 0 ||  section.habenSum != 0)
    {
        const header = document.createElement('h2');
        header.textContent = section.name;
        container.appendChild(header);

        // Create a table for the entries
        const table = document.createElement('table');

        // Only add table headers if there are entries
        if (section.entries && section.entries.length > 0) {
            const thead = document.createElement('thead');
            const headerRow = document.createElement('tr');
            ['Konto', 'Soll-Saldo', 'Haben-Saldo'].forEach(header => {
                const th = document.createElement('th');
                th.textContent = header;
                headerRow.appendChild(th);
            });
            thead.appendChild(headerRow);
            table.appendChild(thead);
            
            const tbody = document.createElement('tbody');
            section.entries.forEach(entry => {
                const row = document.createElement('tr');
                const td = document.createElement('td');
                td.textContent = entry.account;
                row.appendChild(td);
                const td2 = document.createElement('td');
                td2.textContent = parseFloat(entry.sollSaldo).toFixed(2);
                row.appendChild(td2);
                const td3 = document.createElement('td');
                td3.textContent = parseFloat(entry.habenSaldo).toFixed(2);
                row.appendChild(td3);
                tbody.appendChild(row);
            });
            table.appendChild(tbody);
            container.appendChild(table);
        }

        // Recursively render sub-classes
        if (section.subClasses) {
            section.subClasses.forEach(subClass => {
                renderBalanceSheetSection(subClass, container);
            });
        }
    }
}

function renderBalanceSheet(data) {
    const mainElement = document.querySelector('main');
    data.KontenKlassen.forEach(section => {
        renderBalanceSheetSection(section, mainElement);
    });
    const hr = document.createElement('h2');
    mainElement.appendChild(hr);

    const table = document.createElement('table');

    const thead = document.createElement('thead');
    const headerRow = document.createElement('tr');
    ['Bilanzsumme', 'Soll-Saldo', 'Haben-Saldo'].forEach(header => {
         const th = document.createElement('th');
         th.textContent = header;
         headerRow.appendChild(th);
     });
     thead.appendChild(headerRow);
     table.appendChild(thead);
     const tbody = document.createElement('tbody');
     const row = document.createElement('tr');
     const td = document.createElement('td');
     td.textContent = "";
     row.appendChild(td);
     const td2 = document.createElement('td');
     td2.textContent = parseFloat(data.sollSaldo).toFixed(2);
     row.appendChild(td2);
     const td3 = document.createElement('td');
     td3.textContent = parseFloat(data.habenSaldo).toFixed(2);
     row.appendChild(td3);
     tbody.appendChild(row);
     table.appendChild(tbody);
     mainElement.appendChild(table);

}
