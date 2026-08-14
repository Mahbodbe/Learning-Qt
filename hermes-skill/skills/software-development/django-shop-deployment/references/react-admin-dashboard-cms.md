# React Admin Dashboard Build & CMS Expansion Recipe

When updating the React frontend for the admin panel (`admin-frontend/src/App.jsx`):

1. **New Fields in `SiteSetting`**:
   Expand `SiteSetting` in `models.py` with fields like `phone_number`, `email`, `address`, `working_hours`, `telegram_link`, `instagram_link`, `about_title`, `about_text`, `contact_title`, `contact_text`.

2. **React Admin Tabs**:
   Update `SettingsTab` in `App.jsx` with tabs (Home, About, Contact, Footer).
   Ensure form inputs use `<input>` or `<textarea>` directly, or existing custom components. Using an undefined `<Input>` component causes React render crash (blank/dark screen).

3. **Build & Deploy**:
   ```bash
   cd /root/resin-web/admin-frontend
   npm run build
   # Remove old JS bundles to prevent stale chunk serving
   rm -f /root/resin-web/website/staticfiles/dashboard/assets/index-*.js
   cp -r dist/* /root/resin-web/website/staticfiles/dashboard/
   ```

4. **Verify**:
   Reload Gunicorn (`kill -HUP <master_pid>`) and verify `/dashboard/` loads without console errors.
