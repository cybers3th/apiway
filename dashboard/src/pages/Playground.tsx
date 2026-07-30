import { useState } from "react";
import { probe, type ProbeResult } from "../api";
import { useAuth } from "../auth";

const presets = [
  { label: "GET /api/orders", path: "/api/orders" },
  { label: "GET /api/catalog", path: "/api/catalog" },
  { label: "GET /auth/me", path: "/auth/me" },
];

export default function Playground() {
  const { session } = useAuth();
  const [path, setPath] = useState("/api/orders");
  const [withToken, setWithToken] = useState(true);
  const [result, setResult] = useState<ProbeResult | null>(null);
  const [busy, setBusy] = useState(false);

  async function run() {
    setBusy(true);
    try {
      const res = await probe(withToken ? (session?.token ?? null) : null, path);
      setResult(res);
    } finally {
      setBusy(false);
    }
  }

  return (
    <div>
      <h1>Request playground</h1>
      <p className="muted">
        Send requests through the gateway to see authentication, RBAC and rate limiting in action.
      </p>

      <div className="card">
        <div className="row">
          {presets.map((p) => (
            <button key={p.path} className="btn ghost" onClick={() => setPath(p.path)}>
              {p.label}
            </button>
          ))}
        </div>
        <div className="row">
          <input className="grow" value={path} onChange={(e) => setPath(e.target.value)} />
          <label className="checkbox">
            <input
              type="checkbox"
              checked={withToken}
              onChange={(e) => setWithToken(e.target.checked)}
            />
            send token
          </label>
          <button className="btn primary" onClick={run} disabled={busy}>
            {busy ? "Sending…" : "Send"}
          </button>
        </div>
      </div>

      {result && (
        <div className="card">
          <div className={`status ${result.ok ? "ok" : "bad"}`}>HTTP {result.status}</div>
          <pre className="body">{result.body}</pre>
        </div>
      )}
    </div>
  );
}
