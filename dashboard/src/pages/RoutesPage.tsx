import { useEffect, useState } from "react";
import { fetchRoutes, type RouteInfo } from "../api";
import { useAuth } from "../auth";

export default function RoutesPage() {
  const { session } = useAuth();
  const [routes, setRoutes] = useState<RouteInfo[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (!session) return;
    fetchRoutes(session.token)
      .then(setRoutes)
      .catch((e) => setError(e instanceof Error ? e.message : String(e)));
  }, [session]);

  return (
    <div>
      <h1>Routes</h1>
      <p className="muted">Live view of the gateway routing table and its access policy.</p>
      {error && <div className="card notice">{error}</div>}
      <div className="card">
        <table className="data">
          <thead>
            <tr>
              <th>Path prefix</th>
              <th>Upstream</th>
              <th>Auth</th>
              <th>API key</th>
              <th>Required permissions</th>
            </tr>
          </thead>
          <tbody>
            {routes.map((r) => (
              <tr key={r.path_prefix}>
                <td>
                  <code>{r.path_prefix}</code>
                </td>
                <td className="muted">{r.upstream}</td>
                <td>
                  <span className={`pill ${r.auth ? "on" : "off"}`}>
                    {r.auth ? "required" : "public"}
                  </span>
                </td>
                <td>{r.allow_api_key ? "✓" : "—"}</td>
                <td>
                  {r.required_permissions.length
                    ? r.required_permissions.map((p) => (
                        <code key={p} className="perm">
                          {p}
                        </code>
                      ))
                    : "—"}
                </td>
              </tr>
            ))}
            {!routes.length && !error && (
              <tr>
                <td colSpan={5} className="muted">
                  Loading…
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
}
