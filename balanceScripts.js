let accounts = null;

// Function to fetch and sort accounts
function fetchAndSortAccounts(client) {
  return  fetch(`/cgi-bin/clientAccounts.cgi?client=${client}`)
    .then(response => response.json())
    .then(data => {
      // Sort the accounts by the account key
      accounts = data.sort((a, b) => a.accountkey.localeCompare(b.accountkey));
      return accounts; // Return the sorted accounts
    });
}

// Function to get account name by key
function getAccountNameByKey(accountKey) {
  // Find the account with the given key
  const account = accounts.find(acc => acc.accountkey === accountKey);
  return account ? account.accountname : null; // Return the account name or null if not found
}



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
    if (!accounts) fetchAndSortAccounts(client);
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
                let accountName = getAccountNameByKey(entry.account);
                accountName = accountName ? accountName : "";
                td.textContent = entry.account + " " + accountName;
                row.appendChild(td);
                const td2 = document.createElement('td');
                td2.textContent = entry.sollSaldo == 0 ? '' : parseFloat(entry.sollSaldo).toFixed(2);
                row.appendChild(td2);
                const td3 = document.createElement('td');
                td3.textContent = entry.habenSaldo == 0 ? '' : parseFloat(entry.habenSaldo).toFixed(2);
                row.appendChild(td3);
                tbody.appendChild(row);
            });
            const row = document.createElement('tr');
            row.className = 'total-row';
            const td = document.createElement('td');
            td.textContent = '';
            row.appendChild(td);
            const td2 = document.createElement('td');
            td2.textContent = section.sollSum == 0 ? '' :  parseFloat(section.sollSum).toFixed(2);
            row.appendChild(td2);
            const td3 = document.createElement('td');
            td3.textContent = section.habenSum == 0 ? '' : parseFloat(section.habenSum).toFixed(2);
            row.appendChild(td3);
            tbody.appendChild(row);

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

