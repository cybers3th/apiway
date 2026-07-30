import { NavLink, Outlet } from "react-router-dom";
import { useAuth } from "../auth";

const links = [
  { to: "/", label: "Overview", end: true },
  { to: "/routes", label: "Routes" },
  { to: "/playground", label: "Playground" },
];

export default function Layout() {
  const { session, signOut } = useAuth();
  return (
    <div className="app">
      <aside className="sidebar">
        <div className="brand">
          <span className="brand-mark">▤</span> Apiway
        </div>
        <nav>
          {links.map((l) => (
            <NavLink key={l.to} to={l.to} end={l.end} className="nav-link">
              {l.label}
            </NavLink>
          ))}
        </nav>
        <div className="sidebar-footer">
          <div className="who">
            <div className="who-name">{session?.username}</div>
            <div className="who-roles">{session?.roles.join(", ")}</div>
          </div>
          <button className="btn ghost" onClick={signOut}>
            Sign out
          </button>
        </div>
      </aside>
      <main className="content">
        <Outlet />
      </main>
    </div>
  );
}
