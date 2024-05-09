import { useState } from "react";
import { ReactTerminal } from "react-terminal";
import { TerminalContextProvider} from "react-terminal"

function App() {

  const customTheme = {
    themeBGColor: "#fdf6e4",
    themeColor: "#563618",
    themePromptColor: "#cc8139"
  }

  const handler = async(cmd,cmdArg) => {
    const command = `${cmd} ${cmdArg}`
    const url = `/v1/ephemeraldb?command=${command}`
    const data = await fetch(url,{
      method: "GET",
      mode: "same-origin",
      credentials: "same-origin",
    });
    if (data.status === 400 || data.status === 404) {
      const actualData = await data.json();
      return <ResponseInjecttion response={actualData.response} state={true}/>
    }else {
      const actualData = await data.json();
      return <ResponseInjecttion response={actualData.response} state={false}/>
    }
  }

  return (
    <TerminalContextProvider>
    <div className="main-container">
        <ReactTerminal 
          commands = {{
            EphemeralDB : <WhatIsEphemeralDB />
          }}
          welcomeMessage={<WelcomeMessage />}
          prompt="EphemeralDB>"
          showControlBar={false}
          themes={{
            "ephemraldb-custom-theme" : customTheme
          }}
          theme="ephemraldb-custom-theme"
          defaultHandler={handler}
        />
    </div>
    </TerminalContextProvider>
  )
}

function WelcomeMessage() {
  return(
    <div className="welcomemessage-container">
      <div>
        Welcome to <strong>EphemeralDB</strong> playground , a demonstration of the <strong>EphemeralDB</strong> database!
      </div>
      <div>
        Please Type <strong>"EphemeralDB"</strong>
      </div>
    </div>
  )
}

function ResponseInjecttion({response,state}) {
  return(
    <div>
      {
        state ? `(error) ${response}` : `${response}`
      }
    </div>
  )
}


function WhatIsEphemeralDB() {
  return (
    <>
      <h1>EphemeralDB: A Simple, Efficient Key-Value Database</h1>

      <p>EphemeralDB is a key-value database designed for simplicity and efficiency, focusing on storing data directly in main memory for fast access. Unlike traditional databases, EphemeralDB does not enforce any specific data types or formats. Instead, it treats all data as binary, allowing users to define and parse their own data types on the client side.</p>

      <h2>Key Operations:</h2>

      <ul>
        <li>
          <strong>SET:</strong> Store a value against a key. So we can use <strong>SET</strong> command to store the value against the key
          <pre>
            <code>SET car Porsche</code>
          </pre>
        </li>
        <li>
          <strong>GET:</strong> Retrieve the value associated with a key. So we can use <strong>GET</strong> command to retrieve the value against the key
          <pre>
            <code>GET car</code>
          </pre>
        </li>
        <li>
          <strong>UPDATE:</strong> Modify the value associated with a key. So we can use <strong>UPD</strong> command to modify the value against the key
          <pre>
            <code>UPD car Audi</code>
          </pre>
        </li>
        <li>
          <strong>DELETE:</strong> Remove a key-value pair from the database. So we can use <strong>DEL</strong> command to delete the value as well as the key
          <pre>
            <code>DEL car</code>
          </pre>
        </li>
        <li>
          <strong>EXIST:</strong> Check if a key exists in the database. Returns 1 if the key exists, and 0 otherwise.
          <pre>
            <code>EXIST car</code>
          </pre>
        </li>
      </ul>

      <p>EphemeralDB allows developers to define their own data structures and formats on the client side, providing flexibility without limiting the capabilities of the database. While it draws inspiration from Redis, it does not support predefined data structures like strings, lists, hashes, sorted sets, or probabilistic data structures like HyperLogLog.</p>

      <p>With its simplicity and efficiency, EphemeralDB is suitable for applications requiring fast data access and minimal overhead. Whether used for caching, session management, or temporary data storage, EphemeralDB offers a lightweight solution for managing key-value data in memory.</p>

      <p>Even though as of now I didn't implement the TTL for caching, in the future, I will add this. What makes my project different from Redis is that it stores binary data only, and developers need to create their own logic to define data types.</p>
    </>
  );
}

export default App
