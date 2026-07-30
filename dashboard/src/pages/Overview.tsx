import { useEffect, useState } from "react";
import { fetchRoutes, type RouteInfo } from "../api";
import { useAuth } from "../auth";
import StatCard from "../components/StatCard";

export default function Overview() {
  const { session } = useAuth();
  const [routes, setRoutes] = useState<RouteInfo[]>([]);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (!session) return;
    fetchRoutes(session.token)
      .then(setRoutes)
      .catch((e) => setError(e instanceof Error ? e.message : String(e)));
  }, [session]);

  const authed = routes.filter((r) => r.auth).length;

  return (
    <div>
      <h1>Overview</h1>
      <div className="grid">
        <StatCard label="Configured routes" value={routes.length || "—"} />
        <StatCard label="Protected routes" value={routes.length ? authed : "—"} />
        <StatCard label="Your roles" value={session?.roles.length ?? 0} hint={session?.roles.join(", ")} />
        <StatCard
          label="Permissions"
          value={session?.permissions.length ?? 0}
          hint={session?.permissions.join(", ")}
        />
      </div>

      {error && (
        <div className="card notice">
          <strong>Admin API unavailable:</strong> {error}
          <div className="muted">
            The route table requires the <code>admin:read</code> permission. Sign in as an admin
            to view it.
          </div>
        </div>
      )}

      <div className="card">
        <h2>Identity</h2>
        <table className="kv">
          <tbody>
            <tr>
              <td>Username</td>
              <td>{session?.username}</td>
            </tr>
            <tr>
              <td>Roles</td>
              <td>{session?.roles.join(", ") || "—"}</td>
            </tr>
            <tr>
              <td>Permissions</td>
              <td>{session?.permissions.join(", ") || "—"}</td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  );
}
