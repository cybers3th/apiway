import { createContext, useContext, useMemo, useState, type ReactNode } from "react";
import { login as apiLogin, decodeJwt } from "./api";

interface Session {
  token: string;
  username: string;
  roles: string[];
  permissions: string[];
}

interface AuthState {
  session: Session | null;
  signIn: (username: string, password: string) => Promise<void>;
  signOut: () => void;
}

const AuthContext = createContext<AuthState | undefined>(undefined);

const STORAGE_KEY = "apiway.session";

function load(): Session | null {
  const raw = localStorage.getItem(STORAGE_KEY);
  if (!raw) return null;
  try {
    return JSON.parse(raw) as Session;
  } catch {
    return null;
  }
}

export function AuthProvider({ children }: { children: ReactNode }) {
  const [session, setSession] = useState<Session | null>(load);

  const value = useMemo<AuthState>(
    () => ({
      session,
      async signIn(username, password) {
        const res = await apiLogin(username, password);
        const claims = decodeJwt(res.access_token) ?? {};
        const next: Session = {
          token: res.access_token,
          username: (claims.username as string) ?? username,
          roles: res.roles,
          permissions: res.permissions,
        };
        localStorage.setItem(STORAGE_KEY, JSON.stringify(next));
        setSession(next);
      },
      signOut() {
        localStorage.removeItem(STORAGE_KEY);
        setSession(null);
      },
    }),
    [session],
  );

  return <AuthContext.Provider value={value}>{children}</AuthContext.Provider>;
}

export function useAuth(): AuthState {
  const ctx = useContext(AuthContext);
  if (!ctx) throw new Error("useAuth must be used within AuthProvider");
  return ctx;
}
