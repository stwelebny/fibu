let accounts = [];

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

// Export the functions for use in other modules
export { fetchAndSortAccounts, getAccountNameByKey };

