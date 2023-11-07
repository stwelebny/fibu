
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
    displayAllAccountDetails(client);
});


function displayAllAccountDetails(client) {
    // Fetch the list of all accounts
    fetch(`./cgi-bin/balanceList?client=${client}`)
        .then(response => response.json())
        .then(accountsData => {
            // Process each account
            accountsData.forEach(account => {
                // Display account headline with Soll-Saldo and Haben-Saldo
                displayAccountHeadline(account);

                // Fetch and display the details for this account
                fetch(`./cgi-bin/account?client=${client}&accountnumber=${account.Konto}`)
                    .then(response => response.json())
                    .then(accountDetails => {
                        // Now calling displayData with the correct tableId
                        displayData(accountDetails, `accountDetails${account.Konto}`);
                        displaySaldi(account, `accountDetails${account.Konto}`);
                    })
                    .catch(error => {
                        console.error('Fehler beim Abrufen der Kontodetails', error);
                    });
            });
        })
        .catch(error => {
            console.error('Fehler bei der Abfrage der Kontenliste', error);
        });
}

function displayAccountHeadline(account) {
    let accountsContainer = document.getElementById('accountsContainer');
    let mainElement = document.querySelector('main');

    let accountName = getAccountNameByKey(account.Konto);
    accountName = accountName ? accountName : "";
    const accountHeadline = document.createElement('h2');
    accountHeadline.textContent = `Konto: ${account.Konto} ${accountName}`;

    mainElement.appendChild(accountHeadline);

    // Create a table for the account details
    const table = document.createElement('table');
    table.id = `accountDetails${account.Konto}`;
    mainElement.appendChild(table);
}

function formatCurrencyValue(value) {
    return new Intl.NumberFormat('de-AT', {
        minimumFractionDigits: 2,
        maximumFractionDigits: 2
    }).format(value);
}

function displayData(data, tableId) {
    const tableBody = document.getElementById(tableId);
    tableBody.innerHTML = ''; // Clear any previous data

    data.forEach(entry => {
        const tableRow = document.createElement('tr');

        const Belegdatum = entry.Belegdatum.trim();
        const Belegnummer = entry.Belegnummer.trim();
        const Gegenkonto = entry.Gegenkonto.trim();
        const Soll = formatCurrencyValue(entry["Soll"]).trim();
        const Haben = formatCurrencyValue(entry["Haben"]).trim();
        const Text = entry.Text;

        tableRow.innerHTML = `
            <td>${Belegdatum}</td>
            <td>${Belegnummer}</td>
            <td>${Gegenkonto}</td>
            <td style="text-align: right;">${Soll}</td>
            <td style="text-align: right;">${Haben}</td>
            <td>${Text}</td>
        `;

        tableBody.appendChild(tableRow);
    });
}


function displaySaldi(account, tableId) {
    const tableBody = document.getElementById(tableId);
    const Soll = formatCurrencyValue(account["Soll-Saldo"]).trim();
    const Haben = formatCurrencyValue(account["Haben-Saldo"]).trim();
    const tableRow = document.createElement('tr');
    tableRow.innerHTML = `
        <td></td>
        <td></td>
        <td></td>
        <td style="font-weight: bold; text-align: right;">${Soll}</td>
        <td style="font-weight: bold; text-align: right;">${Haben}</td>
        <td></td>
    `;
    tableBody.appendChild(tableRow);
}

