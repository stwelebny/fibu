
function fetchAccounts() {
  // Replace with your actual backend URL
  const client = getCookie('mandant');
  if (!client) {
    alert('Bitte einen Mandanten angeben!');
    return;
  }

  fetch(`/cgi-bin/clientAccounts?client=${client}`)
    .then(response => response.json())
    .then(data => {
      const accountsList = document.getElementById('accountsList');
      accountsList.innerHTML = ''; // Clear current list
      data.forEach(account => {
        // Add each account to the form
        const div = document.createElement('div');
        div.innerHTML = `Kontonummer: <input type="text" value="${account.accountkey}" /> Kontenbezeichnung: <input type="text" value="${account.accountname}" /><br>`;
        accountsList.appendChild(div);
      });
    });
    addEmptyAccountRow();
}

function addEmptyAccountRow() {
  const accountsList = document.getElementById('accountsList');
  const div = document.createElement('div');
  div.innerHTML = `Account Key: <input type="text" class="account-key" /> Account Name: <input type="text" class="account-name" /><br>`;
  accountsList.appendChild(div);
}

function updateAccounts() {
  if (!client) {
    alert('Bitte einen Mandanten angeben!');
    return;
  }
  const accounts = [];
  document.getElementById('accountsList').querySelectorAll('div').forEach(div => {
    const inputs = div.querySelectorAll('input');
    if (keyInput.value && nameInput.value) { // Only add non-empty accounts
      accounts.push({
        accountkey: keyInput.value,
        accountname: nameInput.value
      });
    });
  });

  // Replace with your actual backend URL
  fetch(`/cgi-bin/clientAccounts?client=${client}`, {
    method: 'POST',
    body: JSON.stringify(accounts),
    headers: {
      'Content-Type': 'application/json'
    }
  }).then(response => {
    if (response.ok) {
      alert('Accounts updated successfully');
    } else {
      alert('Failed to update accounts');
    }
  });
}

